/// \file ZmqHelpers.h
///
/// Zmq Helper Classes
///
/// TODO: Write something here
///

#ifndef __MVP_FRONTEND_ZMQHELPERS_H__
#define __MVP_FRONTEND_ZMQHELPERS_H__

#include <google/protobuf/message.h>
#include <string>
#include <zmq.hpp>

namespace mvp {
namespace frontend {

std::string sock_recv(zmq::socket_t& sock);

void sock_send(zmq::socket_t& sock, std::string const& str_message);

void sock_send(zmq::socket_t& sock, google::protobuf::Message const& message);

bool sock_poll(zmq::socket_t& sock, int timeout = -1);

}} // namespace frontend, mvp

#endif
