#include <iostream>

#include <mvp/Frontend/ZmqServerHelper.h>
#include <mvp/Frontend/Session.h>
#include <mvp/Frontend/SessionStatus.h>

#include <vw/Core/Log.h>
#include <vw/Core/Exception.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace vw;
using namespace mvp;
using namespace mvp::frontend;

int main (int argc, char *argv[]) {
  vw_out(vw::InfoMessage, "mvpd") << "Started" << endl;
  zmq::context_t context(1);
  ZmqServerHelper helper(context);

  Session session;
  SessionStatus session_status;

  while (1) {
    ZmqServerHelper::PollEventSet events = helper.poll();

    vector<pipeline::JobDesc> orphaned_jobs = session_status.prune_orphaned_jobs();
    BOOST_FOREACH(pipeline::JobDesc j, orphaned_jobs) {
      vw::vw_out(vw::InfoMessage, "mvpd") << "Orphaned job ID = " << j.id() << std::endl;
      session_status.add_orphan(j);
    }

    if (events.count(ZmqServerHelper::COMMAND_EVENT)) {
      CommandMsg cmd(helper.recv_cmd());
      CommandReplyMsg reply;
      reply.set_cmd(cmd.cmd());

      switch(cmd.cmd()) {
        case CommandMsg::LAUNCH:
          vw_out(vw::InfoMessage, "mvpd") << "CommandMsg::LAUNCH" << endl;
          session.reset(cmd.session());
          session_status.reset(session.num_jobs());
          helper.send_bcast(WorkerCommandMsg::WAKE);
          break;
        case CommandMsg::STATUS:
          vw_out(vw::InfoMessage, "mvpd") << "CommandMsg::STATUS" << endl;
          *reply.mutable_status_report() = session_status.report();
          break;
        case CommandMsg::INFO:
          vw_out(vw::InfoMessage, "mvpd") << "CommandMsg::INFO" << endl;
          // TODO: do something here
          break;
        case CommandMsg::KILL:
          vw_out(vw::InfoMessage, "mvpd") << "CommandMsg::KILL" << endl;
          helper.send_bcast(WorkerCommandMsg::KILL);
          break;
        case CommandMsg::JOB:
          vw_out(vw::InfoMessage, "mvpd") << "CommandMsg::JOB" << endl;
          if (session_status.has_orphans()) {
            *reply.mutable_job() = session_status.next_orphan();
            session_status.add_job(reply.job());
            vw_out(vw::InfoMessage, "mvpd") << "Dispatched orphaned job ID = " << reply.job().id() << endl;
          } else if (session.has_next()) {
            *reply.mutable_job() = session.next();
            session_status.add_job(reply.job());
            vw_out(vw::InfoMessage, "mvpd") << "Dispatched job ID = " << reply.job().id() << endl;
          }
          break;
        default:
          vw_throw(vw::LogicErr() << "Unrecognized command");  
      }
      helper.send_cmd_reply(reply);
    }
    
    if (events.count(ZmqServerHelper::STATUS_EVENT)) {
      vw_out(vw::VerboseDebugMessage, "mvpd") << "ZmqServerHelper::STATUS_EVENT" << endl;
      session_status.update_status(helper.recv_status());

      vector<pipeline::JobDesc> completed_jobs = session_status.prune_completed_jobs();
      BOOST_FOREACH(pipeline::JobDesc j, completed_jobs) {
        vw_out(vw::InfoMessage, "mvpd") << "Completed job ID = " << j.id() << std::endl;
      }
    }
  }

  return 0;
}
