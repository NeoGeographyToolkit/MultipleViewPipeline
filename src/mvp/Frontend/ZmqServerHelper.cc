#include <mvp/Core/Settings.h>

#include <mvp/Frontend/ZmqServerHelper.h>
#include <mvp/Frontend/ZmqHelpers.h>

namespace mvp {
namespace frontend {

ZmqServerHelper::ZmqServerHelper(zmq::context_t& context) : 
  m_cmd_sock(context, ZMQ_REP),
  m_bcast_sock(context, ZMQ_PUB),
  m_status_sock(context, ZMQ_SUB) {

  std::string cmd_sock_url = "tcp://*:" + mvp_settings().ports().command();
  std::string bcast_sock_url = "tcp://*:" + mvp_settings().ports().broadcast();
  std::string status_sock_url = "tcp://*:" + mvp_settings().ports().status();

  m_cmd_sock.bind(cmd_sock_url.c_str());
  m_bcast_sock.bind(bcast_sock_url.c_str());
  m_status_sock.bind(status_sock_url.c_str()); 
}

ZmqServerHelper::PollEventSet ZmqServerHelper::poll() {
  PollEventSet events;

  //  Initialize poll set
  zmq::pollitem_t items [] = {
  { m_cmd_sock, 0, ZMQ_POLLIN, 0 },
  { m_status_sock, 0, ZMQ_POLLIN, 0 }
  };

  zmq::poll(items, 2, -1);

  if (items[0].revents & ZMQ_POLLIN) {
    events.insert(COMMAND_EVENT);
  }
  if (items[1].revents & ZMQ_POLLIN) {
    events.insert(STATUS_EVENT);
  }

  return events;
}

StatusUpdateMsg ZmqServerHelper::recv_status() {
  StatusUpdateMsg update;
  update.ParseFromString(sock_recv(m_status_sock));
  return update;
}

CommandMsg ZmqServerHelper::recv_cmd() {
  CommandMsg cmd;
  cmd.ParseFromString(sock_recv(m_cmd_sock));
  return cmd;
}

void ZmqServerHelper::send_cmd_reply(CommandReplyMsg const& cmd_reply) {
  sock_send(m_cmd_sock, cmd_reply);
}

void ZmqServerHelper::send_bcast(WorkerCommandMsg::CommandType cmd_enum) {
  WorkerCommandMsg bcast_cmd;
  bcast_cmd.set_cmd(cmd_enum);
  sock_send(m_bcast_sock, bcast_cmd);
}

}} // namepace frontend, mvp
