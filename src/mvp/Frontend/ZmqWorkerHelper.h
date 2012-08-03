/// \file ZmqWorkerHelper.h
///
/// Zmq Worker Helper Class
///
/// TODO: Write something here
///

#ifndef __MVP_FRONTEND_ZMQWORKERHELPER_H__
#define __MVP_FRONTEND_ZMQWORKERHELPER_H__

#include <mvp/Frontend/Messages.pb.h>

#include <zmq.hpp>

namespace mvp {
namespace frontend {

class ZmqWorkerHelper {

  zmq::socket_t m_cmd_sock;
  zmq::socket_t m_bcast_sock;
  zmq::socket_t m_status_sock;

  bool m_startup;

  public:
    ZmqWorkerHelper(zmq::context_t& context, std::string const& hostname);

    WorkerCommandMsg recv_bcast();

    CommandReplyMsg get_next_job();
};

}} // namespace frontend, mvp

#endif
