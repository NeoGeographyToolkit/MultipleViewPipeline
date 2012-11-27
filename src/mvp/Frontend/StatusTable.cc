#include <mvp/Frontend/StatusTable.h>

#include <ctime>

#include <mvp/Core/Settings.h>
#include <boost/foreach.hpp>

#include <vw/Core/Log.h>

namespace mvp {
namespace frontend {

void StatusTable::add_entry(pipeline::JobDesc const& job_desc) {
  StatusReport::Entry entry;
  *entry.mutable_job() = job_desc;
  entry.set_status(0.0);
  entry.set_last_seen(time(NULL));
  m_entries[job_desc.id()] = entry;
}

void StatusTable::update_status(StatusUpdateMsg const& status_update) {
  if (m_entries.count(status_update.job_id())) {
    StatusReport::Entry &cursor = m_entries[status_update.job_id()];
    cursor.set_status(status_update.status());
    cursor.set_last_seen(time(NULL));
  } else {
    vw::vw_out(vw::VerboseDebugMessage, "mvpd") << "Got a status update for an unknown job ID = " << status_update.job_id() << std::endl;
  }
}

std::vector<StatusReport::Entry> StatusTable::entries() const {
  std::vector<StatusReport::Entry> result;

  BOOST_FOREACH(EntryMap::value_type const& e, m_entries) {
    result.push_back(e.second);
  }

  return result;
}

std::vector<pipeline::JobDesc> StatusTable::prune_completed_entries() {
  std::vector<pipeline::JobDesc> completed_entries;

  EntryMap::iterator iter = m_entries.begin();
  while (iter != m_entries.end()) {
    StatusReport::Entry &cursor = iter->second;
    if (cursor.status() < 0) {
      completed_entries.push_back(cursor.job());
      m_entries.erase(iter++);
    } else {
      ++iter;
    }
  }

  return completed_entries;
}

std::vector<pipeline::JobDesc> StatusTable::prune_orphaned_entries() {
  time_t curr_time = time(NULL);
  std::vector<pipeline::JobDesc> orphaned_entries;

  EntryMap::iterator iter = m_entries.begin();
  while (iter != m_entries.end()) {
    StatusReport::Entry &cursor = iter->second;
    if (curr_time - cursor.last_seen() > mvp_settings().timeouts().orphan()) {
      orphaned_entries.push_back(cursor.job());
      m_entries.erase(iter++);
    } else {
      ++iter;
    }
  }

  return orphaned_entries;
}

}} // namespace pipeline, mvp
