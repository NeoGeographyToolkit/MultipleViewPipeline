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

StatusReport assemble_status_report(int jobs_completed, int total_jobs, 
                                    std::vector<StatusReport::Entry> const& actives, 
                                    std::vector<pipeline::JobDesc> const& orphans) {

  StatusReport status_report;
  status_report.set_jobs_completed(jobs_completed);
  status_report.set_total_jobs(total_jobs);

  std::copy(actives.begin(), actives.end(), RepeatedFieldBackInserter(status_report.mutable_actives()));
  std::copy(orphans.begin(), orphans.end(), RepeatedFieldBackInserter(status_report.mutable_orphans()));    

  return status_report;
}

int main (int argc, char *argv[]) {
  vw_out(vw::InfoMessage, "mvpd") << "Started" << endl;
  zmq::context_t context(1);
  ZmqServerHelper helper(context);

  Session session;
  SessionStatus session_status;
  std::vector<pipeline::JobDesc> orphaned_jobs;
  int jobs_completed = 0;

  while (1) {
    ZmqServerHelper::PollEventSet events = helper.poll();

    BOOST_FOREACH(pipeline::JobDesc const& j, session_status.prune_orphaned_entries()) {
      vw::vw_out(vw::InfoMessage, "mvpd") << "Orphaned job ID = " << j.id() << std::endl;
      orphaned_jobs.push_back(j);
    }

    if (events.count(ZmqServerHelper::COMMAND_EVENT)) {
      CommandMsg cmd(helper.recv_cmd());
      CommandReplyMsg reply;
      reply.set_cmd(cmd.cmd());

      switch(cmd.cmd()) {
        case CommandMsg::LAUNCH:
          vw_out(vw::InfoMessage, "mvpd") << "CommandMsg::LAUNCH" << endl;

          session.reset(cmd.session());
          session_status.reset();
          orphaned_jobs.clear();
          jobs_completed = 0;

          helper.send_bcast(WorkerCommandMsg::WAKE);
          break;
        case CommandMsg::STATUS:
          vw_out(vw::InfoMessage, "mvpd") << "CommandMsg::STATUS" << endl;
          *reply.mutable_status_report() = assemble_status_report(jobs_completed, session.num_jobs(),
                                                                  session_status.entries(), orphaned_jobs);
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
          if (!orphaned_jobs.empty()) {
            *reply.mutable_job() = orphaned_jobs.back();
            orphaned_jobs.pop_back();
            session_status.add_entry(reply.job());
            vw_out(vw::InfoMessage, "mvpd") << "Dispatched orphaned job ID = " << reply.job().id() << endl;
          } else if (session.has_next()) {
            *reply.mutable_job() = session.next();
            session_status.add_entry(reply.job());
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

      BOOST_FOREACH(pipeline::JobDesc const& j, session_status.prune_completed_entries()) {
        vw_out(vw::InfoMessage, "mvpd") << "Completed job ID = " << j.id() << std::endl;
        ++jobs_completed;
      }
    }
  }

  return 0;
}
