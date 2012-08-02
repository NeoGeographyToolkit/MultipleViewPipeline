#include <iostream>
#include <string>

#include <mvp/Frontend/ZmqHelpers.h>
#include <mvp/Frontend/Session.h>

#include <vw/Core/Log.h>

using namespace std;
using namespace vw;
using namespace mvp;
using namespace mvp::frontend;

int main (int argc, char *argv[]) {
  vw_out(InfoMessage, "mvpd") << "Started" << endl;
  zmq::context_t context(1);
  ZmqServerHelper helper(context);

  Session session;

  while (1) {
    ZmqServerHelper::PollEventSet events = helper.poll();

    if (events.count(ZmqServerHelper::COMMAND_EVENT)) {
      CommandMsg cmd(helper.recv_cmd());
      CommandReplyMsg reply;
      reply.set_cmd(cmd.cmd());

      switch(cmd.cmd()) {
        case CommandMsg::LAUNCH:
          vw_out(InfoMessage, "mvpd") << "CommandMsg::LAUNCH" << endl;
          session.reset(cmd.session());
          helper.send_bcast(WorkerCommandMsg::WAKE);
          break;
        case CommandMsg::STATUS:
          // TODO: do something here
          break;
        case CommandMsg::INFO:
          vw_out(InfoMessage, "mvpd") << "CommandMsg::INFO" << endl;
          // TODO: do something here
          break;
        case CommandMsg::ABORT:
          vw_out(InfoMessage, "mvpd") << "CommandMsg::ABORT" << endl;
          helper.send_bcast(WorkerCommandMsg::ABORT);
          break;
        case CommandMsg::KILL:
          vw_out(InfoMessage, "mvpd") << "CommandMsg::KILL" << endl;
          helper.send_bcast(WorkerCommandMsg::KILL);
          break;
        case CommandMsg::JOB:
          vw_out(InfoMessage, "mvpd") << "CommandMsg::JOB" << endl;
          if (session.has_next()) {
            *reply.mutable_job() = session.next();
            vw_out(InfoMessage, "mvpd") << "Dispatched job ID = " << reply.job().id() << endl;
          }
          break;
        default:
          vw_throw(LogicErr() << "Unrecognized command");  
      }
      helper.send_cmd_reply(reply);
    }
    
    if (events.count(ZmqServerHelper::STATUS_EVENT)) {
      vw_out(VerboseDebugMessage, "mvpd") << "ZmqServerHelper::STATUS_EVENT" << endl;
      //session_status.update_status(helper.recv_status());
    }
  }

  return 0;
}
