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

    void reset(int total_jobs);

    StatusReport report() const;

    void add_job(pipeline::JobDesc const& job_desc);

    void update_status(StatusUpdateMsg const& status_update);

    std::vector<pipeline::JobDesc> prune_completed_jobs();

    bool has_orphans() const { return !m_orphans.empty(); }

    pipeline::JobDesc next_orphan();

    void tick();
};

}} // namespace frontend, mvp

#endif
