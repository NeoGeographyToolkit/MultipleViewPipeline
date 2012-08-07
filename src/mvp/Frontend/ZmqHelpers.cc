#include <mvp/Frontend/ZmqHelpers.h>

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

bool sock_poll(zmq::socket_t& sock, int timeout) {
  zmq::pollitem_t poller[] = {{sock, 0, ZMQ_POLLIN, 0}};
  zmq::poll(poller, 1, timeout);

  if (poller[0].revents & ZMQ_POLLIN) {
    return true;
  } else {
    return false;
  }
}

}} // namespace frontend, mvp
