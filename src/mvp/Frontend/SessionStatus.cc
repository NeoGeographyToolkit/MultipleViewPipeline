#include <mvp/Frontend/SessionStatus.h>

#include <ctime>

#include <mvp/Core/Settings.h>
#include <boost/foreach.hpp>

#include <vw/Core/Log.h>

namespace mvp {
namespace frontend {

void SessionStatus::reset(int total_jobs) {
  m_jobs_completed = 0;
  m_total_jobs = total_jobs;
  m_actives = EntryMap();
  m_orphans = std::vector<pipeline::JobDesc>();
}

StatusReport SessionStatus::report() {
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
void SessionStatus::add_job(pipeline::JobDesc const& job_desc) {
  StatusReport::Entry entry;
  *entry.mutable_job() = job_desc;
  entry.set_status(0.0);
  entry.set_last_seen(time(NULL));
  m_actives[job_desc.id()] = entry;
}

void SessionStatus::update_status(StatusUpdateMsg const& status_update) {
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
      vw::vw_out(vw::InfoMessage, "mvpd") << "Completed job ID = " << iter->second.job().id() << std::endl;
      m_jobs_completed++;
      m_actives.erase(iter++);
    } else {
      ++iter;
    }
  }
}

pipeline::JobDesc SessionStatus::next_orphan() {
  pipeline::JobDesc job_desc(m_orphans.back());
  m_orphans.pop_back();
  return job_desc;
}

void SessionStatus::tick() {
  time_t curr_time = time(NULL);

  EntryMap::iterator iter = m_actives.begin();
  while (iter != m_actives.end()) {
    if (curr_time - iter->second.last_seen() > mvp_settings().timeouts().orphan()) {
      vw::vw_out(vw::InfoMessage, "mvpd") << "Orphaned job ID = " << iter->second.job().id() << std::endl;
      m_orphans.push_back(iter->second.job());
      m_actives.erase(iter++);
    } else {
      ++iter;
    }
  }
}

}} // namespace frontend, mvp
