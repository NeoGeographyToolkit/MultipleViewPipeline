/// \file SessionStatus.h
///
/// SessionStatus class
///

#ifndef __MVP_FRONTEND_SESSIONSTATUS_H__
#define __MVP_FRONTEND_SESSIONSTATUS_H__

#include <map>
#include <queue>

#include <mvp/Pipeline/JobDesc.pb.h>
#include <mvp/Frontend/Messages.pb.h>

#include <boost/foreach.hpp>

namespace mvp {
namespace frontend {

class SessionStatus {
  typedef std::map<int, StatusReport::Entry> EntryMap;
  EntryMap m_actives;
  std::queue<pipeline::JobDesc> m_orphans;

  public:
    SessionStatus() {}

    StatusReport report() {
      StatusReport status_report;
      
      BOOST_FOREACH(EntryMap::value_type &s, m_actives) {
        *status_report.add_actives() = s.second;
      } 

      return status_report;
    }

    void add_job(pipeline::JobDesc const& job_desc) {
      StatusReport::Entry entry;
      *entry.mutable_job() = job_desc;
      entry.set_status(0.0);
      m_actives[job_desc.id()] = entry;
    }

    void update_status(StatusUpdateMsg const& status_update) {
      if (m_actives.count(status_update.job_id())) {
        m_actives[status_update.job_id()].set_status(status_update.status());
      } else {
        vw::vw_throw(vw::LogicErr() << "Got a status update for an unknown job!");
      }
    }

    bool has_orphans() {
      return false;
    }

    pipeline::JobDesc next_orphan() {
      pipeline::JobDesc job_desc(m_orphans.front());
      m_orphans.pop();
      return job_desc;
    }
};

}} // namespace frontend, mvp

#endif
