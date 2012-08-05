#include <mvp/Core/Settings.h>

#include <mvp/Frontend/ZmqWorkerHelper.h>
#include <mvp/Frontend/ZmqHelpers.h>

namespace mvp {
namespace frontend {

void ZmqWorkerHelper::ProgressCallback::report_progress(double progress) const {
  vw::Mutex::Lock lock(m_mutex);
}

void ZmqWorkerHelper::ProgressCallback::report_finished() const {
  vw::Mutex::Lock lock(m_mutex);
}

ZmqWorkerHelper::ZmqWorkerHelper(zmq::context_t& context, std::string const& hostname) :
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

WorkerCommandMsg ZmqWorkerHelper::recv_bcast() const {
  WorkerCommandMsg cmd;

  if (m_startup) {
    cmd.set_cmd(WorkerCommandMsg::WAKE);
    m_startup = false;
  } else {
    cmd.ParseFromString(sock_recv(m_bcast_sock));
  }
  return cmd;
}

CommandReplyMsg ZmqWorkerHelper::get_next_job() const {
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

void ZmqWorkerHelper::send_status(double update) const {
  StatusUpdateMsg status;
  status.set_status(update);
  sock_send(m_status_sock, status);
}

}} // namespace frontend, mvp
