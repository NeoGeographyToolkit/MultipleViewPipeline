#include <iostream>
#include <string>

#include <mvp/ZmqHelpers.h>

#include <mvp/Config.h>
#include <mvp/MVPJobQueue.h>

using namespace std;
using namespace mvp;

int main (int argc, char *argv[]) {
  zmq::context_t context(1);

  ZmqServerHelper helper(context);

  //  Process messages from both sockets
  MVPJobQueue job_queue;
  while (1) {
    ZmqServerHelper::PollEventSet events = helper.poll();

    if (events.count(ZmqServerHelper::COMMAND_EVENT)) {
      MVPCommandMessage cmd(helper.recv_cmd());

      switch(cmd.cmd()) {
        case MVPCommandMessage::LAUNCH:
          job_queue.reset(cmd.conf_file());
          helper.send_bcast(MVPWorkerCommand::WAKE);
          helper.send_cmd();
          break;
        case MVPCommandMessage::STATUS:
          helper.send_cmd(job_queue.status());
          break;
        case MVPCommandMessage::INFO:
          helper.send_cmd();
          break;
        case MVPCommandMessage::ABORT:
          helper.send_bcast(MVPWorkerCommand::ABORT);
          helper.send_cmd();
          break;
        case MVPCommandMessage::KILL:
          helper.send_bcast(MVPWorkerCommand::KILL);
          helper.send_cmd();
          break;
        case MVPCommandMessage::GET_JOB:
          helper.send_cmd(job_queue.next());
          break;
        default:
          vw_throw(vw::LogicErr() << "Unrecognized command");  
      }
    }
    
    if (events.count(ZmqServerHelper::STATUS_EVENT)) {
      job_queue.update_status(helper.recv_status());
    }
  }

  return 0;
}
