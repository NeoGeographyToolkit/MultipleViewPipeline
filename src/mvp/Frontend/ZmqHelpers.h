/// \file ZmqHelpers.h
///
/// Zmq Helper Classes
///
/// TODO: Write something here
///

#ifndef __MVP_FRONTEND_ZMQHELPERS_H__
#define __MVP_FRONTEND_ZMQHELPERS_H__

#include <mvp/Core/Settings.h>
#include <mvp/Frontend/Messages.pb.h>

#include <zmq.hpp>
#include <set>

namespace mvp {
namespace frontend {

std::string sock_recv(zmq::socket_t& sock) {
  zmq::message_t message;
  sock.recv(&message);
  return std::string(static_cast<char*>(message.data()), message.size());
}

void sock_send(zmq::socket_t& sock, std::string const& str_message) {
  zmq::message_t reply(str_message.size());
  memcpy(reply.data(), str_message.c_str(), str_message.size());
  sock.send(reply);
}

void sock_send(zmq::socket_t& sock, google::protobuf::Message const& message) {
  std::string str_message;
  message.SerializeToString(&str_message);
  sock_send(sock, str_message);
}

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

    ZmqServerHelper(zmq::context_t& context) : 
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
  
    PollEventSet poll() {
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

    StatusUpdateMsg recv_status() {
      StatusUpdateMsg update;
      update.ParseFromString(sock_recv(m_status_sock));
      return update;
    }

    CommandMsg recv_cmd() {
      CommandMsg cmd;
      cmd.ParseFromString(sock_recv(m_cmd_sock));
      return cmd;
    }

    void send_cmd_reply(CommandReplyMsg const& cmd_reply) {
      sock_send(m_cmd_sock, cmd_reply);
    }

    void send_bcast(WorkerCommandMsg::CommandType cmd_enum) {
      WorkerCommandMsg bcast_cmd;
      bcast_cmd.set_cmd(cmd_enum);
      sock_send(m_bcast_sock, bcast_cmd);
    }
};

class ZmqWorkerHelper {

  zmq::socket_t m_cmd_sock;
  zmq::socket_t m_bcast_sock;
  zmq::socket_t m_status_sock;

  bool m_startup;

  public:
    ZmqWorkerHelper(zmq::context_t& context, std::string const& hostname) :
      m_cmd_sock(context, ZMQ_REQ),
      m_bcast_sock(context, ZMQ_SUB),
      m_status_sock(context, ZMQ_PUB),
      m_startup(true) {

      std::string cmd_sock_url("tcp://" + hostname + ":" + mvp_settings().ports().command());
      std::string bcast_sock_url("tcp://" + hostname + ":" + mvp_settings().ports().broadcast());
      std::string status_sock_url("tcp://" + hostname + ":" + mvp_settings().ports().status());

      m_cmd_sock.connect(cmd_sock_url.c_str());
      m_bcast_sock.connect(bcast_sock_url.c_str());
      m_status_sock.connect(status_sock_url.c_str());

      // Set subscription filter
      m_bcast_sock.setsockopt(ZMQ_SUBSCRIBE, 0, 0); // Don't filter out any messages
    }

    WorkerCommandMsg recv_bcast() {
      WorkerCommandMsg cmd;

      if (m_startup) {
        cmd.set_cmd(WorkerCommandMsg::WAKE);
        m_startup = false;
      } else {
        cmd.ParseFromString(sock_recv(m_bcast_sock));
      }
      return cmd;
    }

    CommandReplyMsg get_next_job() {
      CommandReplyMsg cmd;
      cmd.set_cmd(CommandMsg::JOB);
      sock_send(m_cmd_sock, cmd);

      // TODO: Write generic single-message poll function
      zmq::pollitem_t cmd_poller[] = {{m_cmd_sock, 0, ZMQ_POLLIN, 0}};
      zmq::poll(cmd_poller, 1, mvp_settings().timeouts().command());

      if (!(cmd_poller[0].revents & ZMQ_POLLIN)) {
        vw_throw(vw::IOErr() << "Lost connection to mvpd");
      }

      CommandReplyMsg reply;
      reply.ParseFromString(sock_recv(m_cmd_sock));

      return reply;
    }
};

}} // namespace frontend, mvp

#endif
