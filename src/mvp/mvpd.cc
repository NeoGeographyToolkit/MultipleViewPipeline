#include <iostream>
#include <string>

#include <zmq.hpp>

#include <mvp/Config.h>
#include <mvp/MVPJobQueue.h>

using namespace std;
using namespace mvp;

const string cmd_sock_url = "tcp://*:" MVP_COMMAND_PORT;
const string bcast_sock_url = "tcp://*:" MVP_BROADCAST_PORT;
const string status_sock_url = "tcp://*:" MVP_STATUS_PORT;

void worker_broadcast(zmq::socket_t& bcast_sock, MVPWorkerCommand::CommandType cmd_enum) {
  MVPWorkerCommand bcast_cmd;
  bcast_cmd.set_cmd(cmd_enum);

  string str_bcast_message;
  bcast_cmd.SerializeToString(&str_bcast_message);
  zmq::message_t bcast_message(str_bcast_message.size());
  memcpy(bcast_message.data(), str_bcast_message.c_str(), str_bcast_message.size());

  bcast_sock.send(bcast_message);
}

int main (int argc, char *argv[]) {
  zmq::context_t context(1);
  
  //  Create sockets
  zmq::socket_t cmd_sock(context, ZMQ_REP);
  zmq::socket_t bcast_sock(context, ZMQ_PUB);
  zmq::socket_t status_sock(context, ZMQ_SUB);

  // Bind sockets
  cmd_sock.bind(cmd_sock_url.c_str());
  bcast_sock.bind(bcast_sock_url.c_str());
  status_sock.bind(status_sock_url.c_str()); 

  //  Initialize poll set
  zmq::pollitem_t items [] = {
  { cmd_sock, 0, ZMQ_POLLIN, 0 },
  { status_sock, 0, ZMQ_POLLIN, 0 }
  };

  //  Process messages from both sockets
  MVPJobQueue job_queue;
  while (1) {
    zmq::poll(items, 2, -1);

    zmq::message_t message; // TODO: Put inside if statements?
    if (items[0].revents & ZMQ_POLLIN) {
      cmd_sock.recv(&message);
      string str_request(static_cast<char*>(message.data()), message.size());

      MVPCommandMessage cmd;
      cmd.ParseFromString(str_request);

      string str_reply;
      switch(cmd.cmd()) {
        case MVPCommandMessage::LAUNCH:
          job_queue.reset(cmd.conf_file());
          worker_broadcast(bcast_sock, MVPWorkerCommand::WAKE);
          break;
        case MVPCommandMessage::STATUS:
          job_queue.status().SerializeToString(&str_reply);
          break;
        case MVPCommandMessage::INFO:
          break;
        case MVPCommandMessage::ABORT:
          worker_broadcast(bcast_sock, MVPWorkerCommand::ABORT);
          break;
        case MVPCommandMessage::KILL:
          worker_broadcast(bcast_sock, MVPWorkerCommand::KILL);
          break;
        case MVPCommandMessage::GET_JOB:
          job_queue.next().SerializeToString(&str_reply);
          break;
        default:
          vw_throw(vw::LogicErr() << "Unrecognized command");  
      }

      zmq::message_t reply(str_reply.size());
      memcpy(reply.data(), str_reply.c_str(), str_reply.size());
      cmd_sock.send(reply);
    }
    if (items[1].revents & ZMQ_POLLIN) {
      status_sock.recv(&message);
      string str_update(static_cast<char*>(message.data()), message.size());

      MVPStatusUpdate update; // TODO: write helper to send/recv protobuffers (templetized)?
      update.ParseFromString(str_update);
      job_queue.update_status(update);
    }
  }

  return 0;
}
