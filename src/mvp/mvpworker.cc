#include <iostream>

#include <mvp/MVPJob.h>
#include <mvp/MVPMessages.pb.h>
#include <mvp/ZmqHelpers.h>

#include <vw/Octave/Main.h>

#include <boost/program_options.hpp>

using namespace vw;
using namespace mvp;
using namespace std;

namespace po = boost::program_options;

struct Options {
  string job_file;
  string hostname;
};

void handle_arguments(int argc, char* argv[], Options *opts) {
  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("job,j", po::value<string>(&opts->job_file), "Jobfile to process")
    ("hostname,n", po::value<string>(&opts->hostname), "Hostname running mvpd")
    ;

  po::positional_options_description p;
  p.add("hostname", 1);

  po::variables_map vm; 
  store(po::command_line_parser(argc, argv).options(cmd_opts).positional(p).run(), vm);

  if (vm.count("help")) {
    vw_throw(vw::ArgumentErr() << cmd_opts);
  }

  if (vm.count("job") + vm.count("hostname") != 1) {
    vw_throw(vw::ArgumentErr() << cmd_opts);
  }

  notify(vm);
}

int main(int argc, char* argv[]) {
  #if MVP_ENABLE_OCTAVE_SUPPORT
  vw::octave::start_octave_interpreter();
  #endif

  Options opts;

  try {
    handle_arguments(argc, argv, &opts);
  } catch (vw::ArgumentErr const& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  if (!opts.job_file.empty()) {
    MVPJob job(MVPJob::load_job_file(opts.job_file));

    MVPTileResult tile_result(job.process_tile(TerminalProgressCallback("mvp", "Processing tile: ")));
   
    write_image(opts.job_file + ".tif", tile_result.alt);

    return 0;
  }

  zmq::context_t context(1);
  ZmqWorkerHelper helper(context, opts.hostname);

  while (1) {
    MVPWorkerBroadcast cmd(helper.recv_bcast());

    switch (cmd.cmd()) {
      case MVPWorkerBroadcast::WAKE:
        cout << "Command wake!" << endl;
        /*
      {
        MVPJob job(helper.get_job());
        job
      } */
        break;
      case MVPWorkerBroadcast::ABORT:
        // Do Nothing
        cout << "Got abort request!" << endl;
        break;
      case MVPWorkerBroadcast::KILL:
        vw_throw(vw::Aborted() << "Kill request");
      default:
        vw_throw(vw::LogicErr() << "Invalid Worker Request");
    }
  }

  // TODO: Make sure this is called every time mvpworker exits...
  #if MVP_ENABLE_OCTAVE_SUPPORT
  do_octave_atexit();
  #endif

  return 0;
}
