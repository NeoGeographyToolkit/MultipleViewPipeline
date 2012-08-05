/// \file ZmqWorkerHelper.h
///
/// Zmq Worker Helper Class
///
/// TODO: Write something here
///

#ifndef __MVP_FRONTEND_ZMQWORKERHELPER_H__
#define __MVP_FRONTEND_ZMQWORKERHELPER_H__

#include <vw/Core/ProgressCallback.h>
#include <mvp/Frontend/Messages.pb.h>

#include <zmq.hpp>

namespace mvp {
namespace frontend {

class ZmqWorkerHelper {

  mutable zmq::socket_t m_cmd_sock;
  mutable zmq::socket_t m_bcast_sock;
  mutable zmq::socket_t m_status_sock;

  mutable bool m_startup;

  public:
    class ProgressCallback : public vw::ProgressCallback {
      ZmqWorkerHelper const& m_helper;
      int m_job_id;
      public:
        ProgressCallback(ZmqWorkerHelper const& helper, int job_id) : m_helper(helper), m_job_id(job_id) {}
        virtual void report_progress(double progress) const;
        virtual void report_finished() const;
    };

    ZmqWorkerHelper(zmq::context_t& context, std::string const& hostname);

    WorkerCommandMsg recv_bcast() const;

    CommandReplyMsg get_next_job() const;

    void send_status(int job_id, double status) const;

    bool abort_requested() const;
};

}} // namespace frontend, mvp

#endif
