#include <zmq.hpp>
#include <string>
#include <iostream>

using namespace std;

string cmd_sock_url = "tcp://*:6677";
string bcast_sock_url = "tcp://*:6678";
string status_sock_url = "tcp://*:6679";

int main (int argc, char *argv[]) {
  zmq::context_t context(1);
  
  //  Create command socket
  zmq::socket_t cmd_sock(context, ZMQ_REP);
  cmd_sock.bind(cmd_sock_url.c_str());

  //  Create broadcast socket
  zmq::socket_t bcast_sock(context, ZMQ_PUB);
  bcast_sock.bind(bcast_sock_url.c_str());

  // Create status socket
  zmq::socket_t status_sock(context, ZMQ_SUB);
  status_sock.bind(status_sock_url.c_str()); 

  //  Initialize poll set
  zmq::pollitem_t items [] = {
  { cmd_sock, 0, ZMQ_POLLIN, 0 },
  { status_sock, 0, ZMQ_POLLIN, 0 }
  };

  //  Process messages from both sockets
  while (1) {
    zmq::message_t message;
    zmq::poll(items, 2, -1);

    if (items[0].revents & ZMQ_POLLIN) {
      cmd_sock.recv(&message);
      string str_message(static_cast<char*>(message.data()), message.size());
      cout << "received command: " << str_message << " size: " << message.size() << endl;

      zmq::message_t reply(2);
      memcpy(reply.data(), "OK", 2);
      cmd_sock.send(reply);
    }
    if (items[1].revents & ZMQ_POLLIN) {
      status_sock.recv(&message);
      string str_message(static_cast<char*>(message.data()), message.size());
      cout << "received status: " << str_message << endl;
    }
  }

  return 0;
}
