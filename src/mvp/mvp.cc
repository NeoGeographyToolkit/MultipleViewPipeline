#include <mvp/mvp.h>

struct Options {
  po::variables_map vm;
  int render_level;
  string gearman_servers;
  bool dry_run;
  BBox2i tile_bbox;
};

void handle_arguments(int argc, char* argv[], Options *opts) {
  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("config-file,f", po::value<string>()->default_value("mvp.conf"), "Specify a pipeline configuration file")
    ("dry-run", "Print information about the workspace and then exit")
    ;

  po::options_description render_opts("Render Options");
  render_opts.add_options()
    ("col-start", po::value<int>(), "Col to start rendering at")
    ("col-end", po::value<int>(), "One past last col to render")
    ("row-start", po::value<int>(), "Row to start rendering at")
    ("row-end", po::value<int>(), "One past last row to render")
    ("render-level", po::value<int>(&opts->render_level)->default_value(-1), "Level to render at")
    ;

  po::options_description mvp_opts;
  mvp_opts.add(MVPWorkspace::program_options()).add(render_opts);

  #if MVP_ENABLE_GEARMAN_SUPPORT
  po::options_description gearman_opts("Gearman Options");
  gearman_opts.add_options()
    ("gearman-servers", po::value<string>(&opts->gearman_servers), "gearmand server list")
    ;
  mvp_opts.add(gearman_opts);
  #endif

  po::options_description all_opts;
  all_opts.add(cmd_opts).add(mvp_opts);

  store(po::command_line_parser(argc, argv).options(all_opts).run(), opts->vm);

  if (opts->vm.count("help")) {
    vw_throw(vw::ArgumentErr() << all_opts);
  }

  ifstream ifs(opts->vm["config-file"].as<string>().c_str());
  if (ifs) {
    store(parse_config_file(ifs, mvp_opts), opts->vm);
  }

  notify(opts->vm);

  #if MVP_ENABLE_GEARMAN_SUPPORT
  if (opts->vm.count("gearman-servers") && opts->vm["platefile-server"].as<string>() == ".") {
    vw_throw(vw::ArgumentErr() << "Error: When using gearman, you must also specify a platefile-server!");
  }
  #endif

  opts->dry_run = opts->vm.count("dry-run");

  if (opts->vm.count("col-start") || opts->vm.count("col-end") || opts->vm.count("row-start") || opts->vm.count("row-end")) {
    if (opts->vm.count("col-start") && opts->vm.count("col-end") && opts->vm.count("row-start") && opts->vm.count("row-end")) {
      opts->tile_bbox.min()[0] = opts->vm["col-start"].as<int>();
      opts->tile_bbox.max()[0] = opts->vm["col-end"].as<int>();
      opts->tile_bbox.min()[1] = opts->vm["row-start"].as<int>();
      opts->tile_bbox.max()[1] = opts->vm["row-end"].as<int>();
    } else {
      vw_throw(vw::ArgumentErr() << "Error: missing col/row start/end");
    }
  }
}

void print_welcome(MVPWorkspace const& work, Options const& opts) {
  cout << boolalpha << endl;
  cout << "-------------------------------------" << endl;
  cout << "Welcome to the Multiple View Pipeline" << endl;
  cout << "-------------------------------------" << endl;
  cout << endl;
  cout << "Number of images loaded = " << work.num_images() << endl;
  cout << " Equal resolution level = " << work.equal_resolution_level() << endl;
  cout << "    Equal density level = " << work.equal_density_level() << endl;
  cout << endl;
  cout << "# Workspace lonlat BBox #" << endl;
  print_bbox_helper(work.lonlat_work_area());
  cout << endl;
  cout << "# Workspace tile BBox (@ level " << opts.render_level << ") #" << endl;
  print_bbox_helper(work.tile_work_area(work.equal_density_level()));
  cout << endl;
  cout << "-------------------------------------" << endl;
  cout << "        Rendering Information" << endl;
  cout << "-------------------------------------" << endl;
  cout << endl;
  cout << "Render level = " << opts.render_level << endl;
  cout << "  Use octave = " << work.user_settings().use_octave() << endl;
  cout << endl;
  cout << "# Render tile BBox #" << endl;
  print_bbox_helper(opts.tile_bbox);
  cout << endl;
  cout << "-------------------------------------" << endl;
  cout << "              Status" << endl;
  cout << "-------------------------------------" << endl;
  cout << endl;
}

int main(int argc, char* argv[])
{
  #if MVP_ENABLE_OCTAVE_SUPPORT
  start_octave_interpreter();
  #endif

  Options opts;

  try {
    handle_arguments(argc, argv, &opts); 
  } catch (const vw::ArgumentErr& e) {
    vw_out() << e.what() << endl;
    return 1;
  } 

  MVPWorkspace work(MVPWorkspace::construct_from_program_options(opts.vm));

  if (opts.render_level < 1) {
    opts.render_level = work.equal_density_level();
  }

  if (opts.tile_bbox.empty()) {
    opts.tile_bbox = work.tile_work_area(opts.render_level);
  }

  print_welcome(work, opts);

  if (opts.dry_run) {
    vw_out() << "Dry run requested. Exiting..." << endl;
    return 0;
  }

  #if MVP_ENABLE_GEARMAN_SUPPORT
  GearmanClientWrapper gclient;
 
  if (!opts.gearman_servers.empty()) {
    try {
      gclient.add_servers(opts.gearman_servers);
      //TODO: Set client timeout?
    } catch (vw::GearmanErr const& e) {
      vw_out() << e.what() << endl;
      return 1;
    }
  }

  // TODO: GearmanTaskList tasks(gclient);
  std::list<gearman_task_st *> tasks;
  #endif


  plate_tunnel(work, opts.tile_bbox, opts.render_level);

  int curr_tile = 0;
  int num_tiles = opts.tile_bbox.width() * opts.tile_bbox.height();

  for (int col = opts.tile_bbox.min().x(); col < opts.tile_bbox.max().x(); col++) {
    for (int row = opts.tile_bbox.min().y(); row < opts.tile_bbox.max().y(); row++) {
      #if MVP_ENABLE_GEARMAN_SUPPORT
      add_task_gearman(gclient, &tasks, work.assemble_job(col, row, opts.render_level), curr_tile, num_tiles, false);
      #else
      do_task_local(work.assemble_job(col, row, opts.render_level), curr_tile, num_tiles, false);
      #endif
      curr_tile++;
    }
  }

  #if MVP_ENABLE_GEARMAN_SUPPORT
  wait_on_gearman_tasks(gclient.client(), tasks, UNTIL_EVERYTHING_IS_DONE);
  #endif

  vw_out() << endl << "Done." << endl << endl;

  #if MVP_ENABLE_OCTAVE_SUPPORT
  do_octave_atexit();
  #endif

  return 0;
}
