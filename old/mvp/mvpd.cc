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
      MVPCommand cmd(helper.recv_cmd());
      MVPCommandReply reply;
      reply.set_cmd(cmd.cmd());

      switch(cmd.cmd()) {
        case MVPCommand::LAUNCH:
          job_queue.reset(cmd.conf_file());
          helper.send_bcast(MVPWorkerBroadcast::WAKE);
          break;
        case MVPCommand::STATUS:
          *reply.mutable_status_report() = job_queue.status();
          break;
        case MVPCommand::INFO:
          // TODO: do something here
          break;
        case MVPCommand::ABORT:
          helper.send_bcast(MVPWorkerBroadcast::ABORT);
          break;
        case MVPCommand::KILL:
          helper.send_bcast(MVPWorkerBroadcast::KILL);
          break;
        case MVPCommand::JOB:
          if (job_queue.has_next()) {
            *reply.mutable_job_request() = job_queue.next();
          }
          break;
        default:
          vw_throw(vw::LogicErr() << "Unrecognized command");  
      }
      helper.send_cmd(reply);
    }
    
    if (events.count(ZmqServerHelper::STATUS_EVENT)) {
      job_queue.update_status(helper.recv_status());
    }
  }

  return 0;
}
