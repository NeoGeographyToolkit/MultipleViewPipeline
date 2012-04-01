#include <iostream>

#include <mvp/MVPJob.h>
#include <mvp/MVPMessages.pb.h>

#include <vw/Octave/Main.h>

#include <boost/program_options.hpp>
#include <zmq.hpp>

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

  // Create socket urls
  string cmd_sock_url("tcp://" + opts.hostname + ":" MVP_COMMAND_PORT);
  string bcast_sock_url("tcp://" + opts.hostname + ":" MVP_BROADCAST_PORT);
  string status_sock_url("tcp://" + opts.hostname + ":" MVP_STATUS_PORT);

  // Create sockets
  zmq::socket_t cmd_sock(context, ZMQ_REQ);
  zmq::socket_t bcast_sock(context, ZMQ_SUB);
  zmq::socket_t status_sock(context, ZMQ_PUB);

  // Connect sockets
  cmd_sock.connect(cmd_sock_url.c_str());
  bcast_sock.connect(bcast_sock_url.c_str());
  status_sock.connect(status_sock_url.c_str());

  // Set subscription filter
  bcast_sock.setsockopt(ZMQ_SUBSCRIBE, 0, 0); // Don't filter out any messages

  MVPWorkerCommand cmd;
  cmd.set_cmd(MVPWorkerCommand::WAKE);
  while (1) {
    switch (cmd.cmd()) {
      case MVPWorkerCommand::WAKE:
        cout << "Command wake!" << endl;
        /*
        {
          MVPCommandMessage cmd;
          cmd.set_cmd(MVPCommandMessage::GET_JOB);
          string str_cmd;
          cmd.SerializeToString(&str_cmd);

          cmd_sock.send(str_cmd)

          zmq::pollitem_t cmd_poller[] = {{cmd_sock, 0, ZMQ_POLLIN, 0}};
          zmq::poll(cmd_poller, 1, 1000);

          if (!(cmd_poller[0].revents & ZMQ_POLLIN)) {
            vw_throw(vw::IOErr() << "Didn't receive reply from mvpd");
          }
        } */
        break;
      case MVPWorkerCommand::ABORT:
        // Do Nothing
        cout << "Got abort request!" << endl;
        break;
      case MVPWorkerCommand::KILL:
        vw_throw(vw::Aborted() << "Kill request");
      default:
        vw_throw(vw::LogicErr() << "Invalid Worker Request");
    }

    zmq::message_t message;
    bcast_sock.recv(&message);
    string str_message(static_cast<char*>(message.data()), message.size());
    cmd.ParseFromString(str_message);
  }

  // TODO: Make sure this is called every time mvpworker exits...
  #if MVP_ENABLE_OCTAVE_SUPPORT
  do_octave_atexit();
  #endif

  return 0;
}
