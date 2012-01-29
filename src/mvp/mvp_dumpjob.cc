#include <iostream>

#include <mvp/MVPWorkspace.h>
#include <mvp/MVPJob.h>

#include <boost/program_options.hpp>

using namespace vw;
using namespace mvp;
using namespace std;

namespace po = boost::program_options;

struct Options {
  po::variables_map vm;
  int col;
  int row;
  int level;
};

void handle_arguments(int argc, char* argv[], Options *opts) {
  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("config-file,f", po::value<string>()->default_value("mvp.conf"), "Specify a pipeline configuration file")
    ("col,c", po::value<int>(&opts->col)->required(), "Column of tile to dump")
    ("row,r", po::value<int>(&opts->row)->required(), "When dumping a jobfile, row of tile to dump")
    ("level,l", po::value<int>(&opts->level)->required(), "When dumping a jobfile or printing the workspace, level to operate at")
    ;

  po::options_description render_opts("Render Options");
  render_opts.add_options()
    ("col-start", po::value<int>(), "Col to start rendering at")
    ("col-end", po::value<int>(), "One past last col to render")
    ("row-start", po::value<int>(), "Row to start rendering at")
    ("row-end", po::value<int>(), "One past last row to render")
    ("render-level", po::value<int>()->default_value(-1), "Level to render at")
    ;

  po::options_description mvp_opts;
  mvp_opts.add(MVPWorkspace::program_options()).add(render_opts);

  po::options_description all_opts;
  all_opts.add(cmd_opts).add(mvp_opts);

  po::positional_options_description p;
  p.add("col", 1).add("row", 1).add("level", 1);
 
  store(po::command_line_parser(argc, argv).options(all_opts).positional(p).run(), opts->vm);

  if (opts->vm.count("help")) {
    vw_throw(vw::ArgumentErr() << all_opts);
  }

  ifstream ifs(opts->vm["config-file"].as<string>().c_str());
  if (ifs) {
    store(parse_config_file(ifs, mvp_opts), opts->vm);
  }

  notify(opts->vm);
}

int main(int argc, char* argv[])
{
  Options opts;

  try {
    handle_arguments(argc, argv, &opts);
  } catch (vw::ArgumentErr const& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  MVPWorkspace work(MVPWorkspace::construct_from_program_options(opts.vm)); 

  string job_filename = save_job_file(work.assemble_job(opts.col, opts.row, opts.level));

  vw_out() << endl << "Saved: " << job_filename << endl << endl;

  return 0;
}
