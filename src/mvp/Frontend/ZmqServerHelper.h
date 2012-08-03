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
  zmq::socket_t m_cmd_sock;
  zmq::socket_t m_bcast_sock;
  zmq::socket_t m_status_sock;

  public:
    enum PollEvent {
      STATUS_EVENT,
      COMMAND_EVENT,
    };
    typedef std::set<PollEvent> PollEventSet;

    ZmqServerHelper(zmq::context_t& context);

    PollEventSet poll();

    StatusUpdateMsg recv_status();

    CommandMsg recv_cmd();

    void send_cmd_reply(CommandReplyMsg const& cmd_reply);

    void send_bcast(WorkerCommandMsg::CommandType cmd_enum);
};

}} // namespace frontend, mvp

#endif

