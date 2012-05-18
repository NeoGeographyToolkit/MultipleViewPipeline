/// \file ZmqHelpers.h
///
/// Zmq Helper Classes
///
/// TODO: Write something here
///

#ifndef __MVP_ZMQHELPERS_H__
#define __MVP_ZMQHELPERS_H__

#include <mvp/Config.h>
#include <mvp/MVPJobRequest.pb.h>
#include <mvp/MVPMessages.pb.h>

#include <zmq.hpp>
#include <set>

/*
// Get out of settings later
#define MVP_COMMAND_PORT "6677"
#define MVP_STATUS_PORT "6678"
#define MVP_BROADCAST_PORT "6679"
*/

namespace mvp {

const std::string cmd_sock_url = "tcp://*:" MVP_COMMAND_PORT;
const std::string bcast_sock_url = "tcp://*:" MVP_BROADCAST_PORT;
const std::string status_sock_url = "tcp://*:" MVP_STATUS_PORT;

class ZmqServerHelper {

  zmq::socket_t m_cmd_sock;
  zmq::socket_t m_bcast_sock;
  zmq::socket_t m_status_sock;

  void sock_send(zmq::socket_t& sock, std::string const& str_message) {
    zmq::message_t reply(str_message.size());
    memcpy(reply.data(), str_message.c_str(), str_message.size());
    sock.send(reply);
  }

  std::string sock_recv(zmq::socket_t& sock) {
    zmq::message_t message;
    sock.recv(&message);
    return std::string(static_cast<char*>(message.data()), message.size());
  }
  
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

    MVPStatusUpdate recv_status() {
      MVPStatusUpdate update;
      update.ParseFromString(sock_recv(m_status_sock));
      return update;
    }

    MVPCommandMessage recv_cmd() {
      MVPCommandMessage cmd;
      cmd.ParseFromString(sock_recv(m_cmd_sock));
      return cmd;
    }

    void send_cmd(MVPStatusReport const& status) {
      std::string str_cmd_message;
      status.SerializeToString(&str_cmd_message);
      sock_send(m_cmd_sock, str_cmd_message);
    }

    void send_cmd(MVPJobRequest const& job) {
      std::string str_cmd_message;
      job.SerializeToString(&str_cmd_message);
      sock_send(m_cmd_sock, str_cmd_message);
    }

    void send_cmd() {
      sock_send(m_cmd_sock, "");
    }

    void send_bcast(MVPWorkerCommand::CommandType cmd_enum) {
      MVPWorkerCommand bcast_cmd;
      bcast_cmd.set_cmd(cmd_enum);

      std::string str_bcast_message;
      bcast_cmd.SerializeToString(&str_bcast_message);
      sock_send(m_bcast_sock, str_bcast_message);
    }
};

} // namespace mvp

#endif
