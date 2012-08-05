#include <iostream>
#include <csignal>

#include <mvp/Frontend/ZmqWorkerHelper.h>

#include <vw/Octave/Main.h>
#include <vw/Core/Log.h>
#include <vw/Core/Exception.h>

#include <boost/program_options.hpp>

using namespace std;
using namespace vw;
using namespace mvp;
using namespace mvp::frontend;

namespace po = boost::program_options;

void exit_cleanup(){
  vw_out(vw::InfoMessage, "mvpworker") << "Shutting Down" << endl;

  #if MVP_ENABLE_OCTAVE_SUPPORT
  do_octave_atexit();
  #endif
}

void signal_handler(int s) {
  exit_cleanup();
  exit(0);
}

void register_signals() {
  struct sigaction handler;

  handler.sa_handler = signal_handler;
  sigemptyset(&handler.sa_mask);
  handler.sa_flags = 0;

  sigaction(SIGINT, &handler, NULL);
}

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

#include <unistd.h> // usleep

void launch_job(ProgressCallback const& progress, pipeline::JobDesc const& job_desc) {
  for (int i = 0; i < 100; i++) {
    usleep(100000);
    progress.report_fractional_progress(i, 100);
  }
  progress.report_finished();
}

int main(int argc, char* argv[]) {
  #if MVP_ENABLE_OCTAVE_SUPPORT
  octave::start_octave_interpreter();
  #endif

  register_signals();

  Options opts;

  try {
    handle_arguments(argc, argv, &opts);
  } catch (vw::ArgumentErr const& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  /*
  if (!opts.job_file.empty()) {
    MVPJob job(MVPJob::load_job_file(opts.job_file));

    MVPTileResult tile_result(job.process_tile(TerminalProgressCallback("mvp", "Processing tile: ")));
   
    write_image(opts.job_file + ".tif", tile_result.alt);

    return 0;
  }
  */

  vw_out(vw::InfoMessage, "mvpworker") << "Started" << endl;
  zmq::context_t context(1);
  ZmqWorkerHelper helper(context, opts.hostname);

  while (1) {
    WorkerCommandMsg cmd(helper.recv_bcast());

    switch (cmd.cmd()) {
      case WorkerCommandMsg::WAKE:
        vw_out(vw::InfoMessage, "mvpworker") << "WorkerCommandMsg::WAKE" << endl;
        while (1) {
          CommandReplyMsg reply(helper.get_next_job());
          if (reply.has_job()) {
            vw_out(vw::InfoMessage, "mvpworker") << "Working on job ID = " << reply.job().id() << endl;
            try {
              launch_job(ZmqWorkerHelper::ProgressCallback(helper, reply.job().id()), reply.job());
            } catch (vw::Aborted &e) {
              vw_out(vw::InfoMessage, "mvpworker") << "Aborted job ID = " << reply.job().id() << endl;
              break;
            }
          } else {
            break;
          }
        }
        break;
      case WorkerCommandMsg::ABORT:
        vw_out(vw::InfoMessage, "mvpworker") << "WorkerCommandMsg::ABORT" << endl;
        break;
      case WorkerCommandMsg::KILL:
        vw_out(vw::InfoMessage, "mvpworker") << "WorkerCommandMsg::KILL" << endl;
        raise(SIGINT);
      default:
        vw_throw(vw::LogicErr() << "Invalid Worker Request");
    }
  }

  exit_cleanup();

  return 0;
}
