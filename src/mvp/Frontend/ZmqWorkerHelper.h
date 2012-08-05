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
    ZmqWorkerHelper(zmq::context_t& context, std::string const& hostname);

    WorkerCommandMsg recv_bcast() const;

    CommandReplyMsg get_next_job() const;

    void send_status(double status) const;
};

}} // namespace frontend, mvp

#endif
