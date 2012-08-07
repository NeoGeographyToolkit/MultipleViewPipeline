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

StatusReport SessionStatus::report() const {
  StatusReport status_report;
  status_report.set_jobs_completed(m_jobs_completed);
  status_report.set_total_jobs(m_total_jobs);
  
  BOOST_FOREACH(EntryMap::value_type const& s, m_actives) {
    *status_report.add_actives() = s.second;
  } 

  BOOST_FOREACH(pipeline::JobDesc const& j, m_orphans) {
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
    vw::vw_out(vw::VerboseDebugMessage, "mvpd") << "Got a status update for an unknown job ID = " << status_update.job_id() << std::endl;
  }
}

std::vector<pipeline::JobDesc> SessionStatus::prune_completed_jobs() {
  std::vector<pipeline::JobDesc> completed_jobs;

  EntryMap::iterator iter = m_actives.begin();
  while (iter != m_actives.end()) {
    StatusReport::Entry &cursor = iter->second;
    if (cursor.status() < 0) {
      m_jobs_completed++;
      completed_jobs.push_back(cursor.job());
      m_actives.erase(iter++);
    } else {
      ++iter;
    }
  }

  return completed_jobs;
}

std::vector<pipeline::JobDesc> SessionStatus::prune_orphaned_jobs() {
  time_t curr_time = time(NULL);
  std::vector<pipeline::JobDesc> orphaned_jobs;

  EntryMap::iterator iter = m_actives.begin();
  while (iter != m_actives.end()) {
    StatusReport::Entry &cursor = iter->second;
    if (curr_time - cursor.last_seen() > mvp_settings().timeouts().orphan()) {
      orphaned_jobs.push_back(cursor.job());
      m_actives.erase(iter++);
    } else {
      ++iter;
    }
  }

  return orphaned_jobs;
}

pipeline::JobDesc SessionStatus::next_orphan() {
  pipeline::JobDesc job_desc(m_orphans.back());
  m_orphans.pop_back();
  return job_desc;
}

}} // namespace frontend, mvp
