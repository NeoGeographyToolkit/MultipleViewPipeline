#include <iostream>

#include <mvp/MVPWorkspace.h>
#include <mvp/MVPJob.h>

#include <boost/program_options.hpp>

using namespace vw;
using namespace mvp;
using namespace std;

namespace po = boost::program_options;

struct Options {
  string config_file;
  int col;
  int row;
  int level;
};

void handle_arguments(int argc, char* argv[], Options *opts) {
  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("config-file,f", po::value<string>(&opts->config_file)->required(), "Specify a pipeline configuration file")
    ("col,c", po::value<int>(&opts->col)->required(), "Column of tile to dump")
    ("row,r", po::value<int>(&opts->row)->required(), "When dumping a jobfile, row of tile to dump")
    ("level,l", po::value<int>(&opts->level)->required(), "When dumping a jobfile or printing the workspace, level to operate at")
    ;

  po::positional_options_description p;
  p.add("col", 1).add("row", 1).add("level", 1);

  po::variables_map vm; 
  store(po::command_line_parser(argc, argv).options(cmd_opts).positional(p).run(), vm);

  if (vm.count("help")) {
    vw_throw(vw::ArgumentErr() << cmd_opts);
  }

  notify(vm);
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

  MVPWorkspace work(work_request_from_conf(opts.config_file));
  MVPJob job(work.assemble_job(opts.col, opts.row, opts.level));

  string job_filename = job.save_job_file();

  vw_out() << endl << "Saved: " << job_filename << endl << endl;

  return 0;
}
