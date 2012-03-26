#include <iostream>

#include <mvp/MVPJob.h>

#include <boost/program_options.hpp>

using namespace vw;
using namespace mvp;
using namespace std;

namespace po = boost::program_options;

struct Options {
  string job_file;
};

void handle_arguments(int argc, char* argv[], Options *opts) {
  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("job,j", po::value<string>(&opts->job_file), "Jobfile to process")
    ;

  po::positional_options_description p;
  p.add("job", 1);

  po::variables_map vm; 
  store(po::command_line_parser(argc, argv).options(cmd_opts).positional(p).run(), vm);

  if (vm.count("help")) {
    vw_throw(vw::ArgumentErr() << cmd_opts);
  }

  notify(vm);
}

int main(int argc, char* argv[]) {
  Options opts;

  try {
    handle_arguments(argc, argv, &opts);
  } catch (vw::ArgumentErr const& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  MVPJob job(MVPJob::load_job_file(opts.job_file));

  MVPTileResult tile_result(job.process_tile(TerminalProgressCallback("mvp", "Processing tile: ")));
 
  write_image(opts.job_file + ".tif", tile_result.alt);

  return 0;
}
