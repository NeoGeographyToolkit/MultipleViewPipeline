/// \file ZmqServerHelper.h
///
/// Zmq Helper Classes
///
/// TODO: Write something here
///

#ifndef __MVP_FRONTEND_ZMQSERVERHELPER_H__
#define __MVP_FRONTEND_ZMQSERVERHELPER_H__

#include <mvp/Frontend/Messages.pb.h>

#include <zmq.hpp>
#include <set>

namespace mvp {
namespace frontend {

class ZmqServerHelper {
  mutable zmq::socket_t m_cmd_sock;
  mutable zmq::socket_t m_bcast_sock;
  mutable zmq::socket_t m_status_sock;

  public:
    enum PollEvent {
      STATUS_EVENT,
      COMMAND_EVENT,
    };
    typedef std::set<PollEvent> PollEventSet;

    ZmqServerHelper(zmq::context_t& context);

    PollEventSet poll() const;

    StatusUpdateMsg recv_status() const;

    CommandMsg recv_cmd() const;

    void send_cmd_reply(CommandReplyMsg const& cmd_reply) const;

    void send_bcast(WorkerCommandMsg::CommandType cmd_enum) const;
};

}} // namespace frontend, mvp

#endif

