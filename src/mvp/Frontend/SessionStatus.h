/// \file SessionStatus.h
///
/// SessionStatus class
///

#ifndef __MVP_FRONTEND_SESSIONSTATUS_H__
#define __MVP_FRONTEND_SESSIONSTATUS_H__

#include <ctime>

#include <map>
#include <queue>

#include <mvp/Pipeline/JobDesc.pb.h>
#include <mvp/Frontend/Messages.pb.h>
#include <mvp/Core/Settings.h>

#include <boost/foreach.hpp>

namespace mvp {
namespace frontend {

class SessionStatus {
  typedef std::map<int, StatusReport::Entry> EntryMap;
  EntryMap m_actives;
  std::vector<pipeline::JobDesc> m_orphans;
  int m_jobs_completed;
  int m_total_jobs;

  public:
    SessionStatus() : m_jobs_completed(0), m_total_jobs(0) {}

    void reset(int total_jobs) {
      m_jobs_completed = 0;
      m_total_jobs = total_jobs;
      m_actives = EntryMap();
      m_orphans = std::vector<pipeline::JobDesc>();
    }

    StatusReport report() {
      StatusReport status_report;
      status_report.set_jobs_completed(m_jobs_completed);
      status_report.set_total_jobs(m_total_jobs);
      
      BOOST_FOREACH(EntryMap::value_type &s, m_actives) {
        *status_report.add_actives() = s.second;
      } 

      BOOST_FOREACH(pipeline::JobDesc &j, m_orphans) {
        *status_report.add_orphans() = j;
      }

      return status_report;
    }

    void add_job(pipeline::JobDesc const& job_desc) {
      StatusReport::Entry entry;
      *entry.mutable_job() = job_desc;
      entry.set_status(0.0);
      entry.set_last_seen(time(NULL));
      m_actives[job_desc.id()] = entry;
    }

    void update_status(StatusUpdateMsg const& status_update) {
      if (m_actives.count(status_update.job_id())) {
        StatusReport::Entry &cursor = m_actives[status_update.job_id()];
        cursor.set_status(status_update.status());
        cursor.set_last_seen(time(NULL));
      } else {
        vw::vw_throw(vw::LogicErr() << "Got a status update for an unknown job!");
      }

      // prune jobs
      EntryMap::iterator iter = m_actives.begin();
      while (iter != m_actives.end()) {
        if (iter->second.status() < 0) {
          //TODO: vw_out(vw::InfoMessage, "mvpd") << "Completed job ID = " << iter->second.job().id() << std::endl;
          m_jobs_completed++;
          m_actives.erase(iter++);
        } else {
          ++iter;
        }
      }
    }

    bool has_orphans() {
      return !m_orphans.empty();
    }

    pipeline::JobDesc next_orphan() {
      pipeline::JobDesc job_desc(m_orphans.back());
      m_orphans.pop_back();
      return job_desc;
    }

    void tick() {
      time_t curr_time = time(NULL);

      EntryMap::iterator iter = m_actives.begin();
      while (iter != m_actives.end()) {
        if (curr_time - iter->second.last_seen() > mvp_settings().timeouts().orphan()) {
          //TODO: vw_out(vw::InfoMessage, "mvpd") << "Orphaned job ID = " << iter->second.job().id() << std::endl;
          m_orphans.push_back(iter->second.job());
          m_actives.erase(iter++);
        } else {
          ++iter;
        }
      }
    }
};

}} // namespace frontend, mvp

#endif
