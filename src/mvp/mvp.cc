#include <mvp/mvp.h>

int main(int argc, char* argv[])
{
  #if MVP_ENABLE_OCTAVE_SUPPORT
  start_octave_interpreter();
  #endif

  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("silent", "Run without outputting status")
    ("config-file,f", po::value<string>()->default_value("mvp.conf"), "Specify a pipeline configuration file")
    ("print-workspace,p", "Print information about the workspace and exit")
    ("dump-job", "Dump a jobfile")
    ("col,c", po::value<int>(), "When dumping a jobfile, column of tile to dump")
    ("row,r", po::value<int>(), "When dumping a jobfile, row of tile to dump")
    ("level,l", po::value<int>(), "When dumping a jobfile or printing the workspace, level to operate at")
    ("job", po::value<string>(), "Render a jobfile")
    ;

  po::options_description render_opts("Render Options");
  render_opts.add_options()
    ("col-start", po::value<int>(), "Col to start rendering at")
    ("col-end", po::value<int>(), "One past last col to render")
    ("row-start", po::value<int>(), "Row to start rendering at")
    ("row-end", po::value<int>(), "One past last row to render")
    ("render-level", po::value<int>(), "Level to render at")
    ;

  po::options_description mvp_opts;
  mvp_opts.add(MVPWorkspace::program_options()).add(render_opts);

  po::options_description all_opts;
  all_opts.add(cmd_opts).add(mvp_opts);

  po::variables_map vm;
  store(po::command_line_parser(argc, argv).options(all_opts).run(), vm);

  if (vm.count("help")) {
    cout << all_opts << endl;
    return 0;
  }

  ifstream ifs(vm["config-file"].as<string>().c_str());
  if (ifs) {
    store(parse_config_file(ifs, mvp_opts), vm);
  }

  notify(vm);

  MVPWorkspace work(MVPWorkspace::construct_from_program_options(vm)); 

  if (!vm.count("silent")) {
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
    cout << "# Workspace tile BBox (@ equal density level) #" << endl;
    print_bbox_helper(work.tile_work_area(work.equal_density_level()));
    if (vm.count("level")) {
      int print_level = vm["level"].as<int>();
      cout << endl;
      cout << "# Workspace tile BBox (@ level " << print_level << ") #" << endl;
      print_bbox_helper(work.tile_work_area(print_level));
    }
    cout << endl;
  } 

  if (vm.count("print-workspace")) {
    return 0;
  }

  if (vm.count("dump-job")) {
    if (!vm.count("col") || !vm.count("row") || !vm.count("level")) {
      cerr << "Error: Must specify a col, row, and level to dump" << endl;
      return 1;
    }
    save_job_file(work.assemble_job(vm["col"].as<int>(), vm["row"].as<int>(), vm["level"].as<int>()));
    return 0;
  }

  int render_level = work.equal_density_level();
  if (vm.count("render-level")) {
    render_level = vm["render-level"].as<int>();
  }

  BBox2i tile_bbox(work.tile_work_area(render_level));

  if (vm.count("col-start")) {
    VW_ASSERT(vm.count("col-end"), ArgumentErr() << "col-start specified, but col-end not");
    tile_bbox.min()[0] = vm["col-start"].as<int>();
    tile_bbox.max()[0] = vm["col-end"].as<int>();
  }

  if (vm.count("row-start")) {
    VW_ASSERT(vm.count("row-end"), ArgumentErr() << "row-start specified, but col-end not");
    tile_bbox.min()[1] = vm["row-start"].as<int>();
    tile_bbox.max()[1] = vm["row-end"].as<int>();
  }

  if (!vm.count("silent")) {
    cout << "-------------------------------------" << endl;
    cout << "        Rendering Information" << endl;
    cout << "-------------------------------------" << endl;
    cout << endl;
    cout << "Render level = " << render_level << endl;
    cout << "  Use octave = " << vm["use-octave"].as<bool>() << endl;
    cout << endl;
    cout << "# Render tile BBox #" << endl;
    print_bbox_helper(tile_bbox);
    cout << endl;
    cout << "-------------------------------------" << endl;
    cout << "              Status" << endl;
    cout << "-------------------------------------" << endl;
    cout << endl;
  }

  if (vm.count("job")) {
    MVPTileResult result = mvpjob_process_tile(vm["job"].as<string>(), TerminalProgressCallback("mvp", "Processing job: "));
    write_georeferenced_image(vm["job"].as<string>() + ".tif", result.alt, result.georef);
  } else {
    plate_tunnel(work, tile_bbox, render_level);

    int curr_tile = 0;
    int num_tiles = tile_bbox.width() * tile_bbox.height();
    bool gearman = false;

    for (int col = tile_bbox.min().x(); col < tile_bbox.max().x(); col++) {
      for (int row = tile_bbox.min().y(); row < tile_bbox.max().y(); row++) {
        if (gearman) {
//          add_gearman_task(work.assemble_job(col, row, render_level), curr_tile, num_tiles, vm.count("silent"));
        } else {
            add_nongearman_task(work.assemble_job(col, row, render_level), curr_tile, num_tiles, vm.count("silent"));
        }
        curr_tile++;
      }
    }
  }

  if (!vm.count"silent") {
    cout << endl << "Done." << endl;
  }

  #if MVP_ENABLE_OCTAVE_SUPPORT
  do_octave_atexit();
  #endif

  return 0;
}
