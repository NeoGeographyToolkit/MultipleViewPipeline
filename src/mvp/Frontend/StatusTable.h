/// \file StatusTable.h
///
/// StatusTable class
///

#ifndef __MVP_FRONTEND_STATUSTABLE_H__
#define __MVP_FRONTEND_STATUSTABLE_H__

#include <map>
#include <queue>

#include <mvp/Pipeline/JobDesc.pb.h>
#include <mvp/Frontend/Messages.pb.h>

namespace mvp {
namespace frontend {

class StatusTable {
  typedef std::map<int, StatusReport::Entry> EntryMap;
  EntryMap m_entries;

  public:
    StatusTable() : m_entries() {}

    void reset() { m_entries.clear(); }

    void add_entry(pipeline::JobDesc const& job_desc);

    void update_status(StatusUpdateMsg const& status_update);

    std::vector<StatusReport::Entry> entries() const;

    std::vector<pipeline::JobDesc> prune_completed_entries();

    std::vector<pipeline::JobDesc> prune_orphaned_entries();
};

}} // namespace frontend, mvp

#endif
