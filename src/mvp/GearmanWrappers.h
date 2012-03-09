#ifndef __MVP_GEARMAN_WRAPPERS_H__
#define __MVP_GEARMAN_WRAPPERS_H__

#include <mvp/MVPJobRequest.pb.h>
#include <libgearman/gearman.h>

namespace vw {
  VW_DEFINE_EXCEPTION(GearmanErr, Exception);
}

class GearmanClientWrapper {
  gearman_client_st *m_client;
  bool m_is_connected;

  public:
    GearmanClientWrapper() : m_is_connected(false) {
      m_client = gearman_client_create(NULL);
      if (!m_client) {
        throw std::bad_alloc();
      }
    }

    GearmanClientWrapper(GearmanClientWrapper const& other)
      : m_is_connected(other.m_is_connected) {
      m_client = gearman_client_clone(NULL, other.m_client);
      if (!m_client) {
        throw std::bad_alloc();
      }
    }

    GearmanClientWrapper &operator=(GearmanClientWrapper const& other) {
      m_is_connected = other.m_is_connected;
      m_client = gearman_client_clone(NULL, other.m_client);
      if (!m_client) {
        throw std::bad_alloc();
      }
      return *this;
    }

    void add_servers(std::string const& servers) {
      gearman_return_t ret;
      ret = gearman_client_add_servers(m_client, servers.c_str());
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << gearman_client_error(m_client));
      }
      m_is_connected = true;
    }

    bool is_connected() const {
      return m_is_connected;
    }

    gearman_client_st *client() const {
      return m_client;
    }
  
    ~GearmanClientWrapper() {
      gearman_client_free(m_client);
    }
};

class GearmanWorkerWrapper {
  gearman_worker_st *m_worker;
  bool m_is_connected;

  public:
    GearmanWorkerWrapper() : m_is_connected(false) {
      m_worker = gearman_worker_create(NULL);
      if (!m_worker) {
        throw std::bad_alloc();
      }
    }

    GearmanWorkerWrapper(GearmanWorkerWrapper const& other)
      : m_is_connected(other.m_is_connected) {
      m_worker = gearman_worker_clone(NULL, other.m_worker);
      if (!m_worker) {
        throw std::bad_alloc();
      }
    }

    GearmanWorkerWrapper &operator=(GearmanWorkerWrapper const& other) {
      m_is_connected = other.m_is_connected;
      m_worker = gearman_worker_clone(NULL, other.m_worker);
      if (!m_worker) {
        throw std::bad_alloc();
      }
      return *this;
    }

    void add_servers(std::string const& servers) {
      gearman_return_t ret;
      ret = gearman_worker_add_servers(m_worker, servers.c_str());
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << gearman_worker_error(m_worker));
      }
      m_is_connected = true;
    }

    void add_function(std::string const& function_name, gearman_worker_fn *function) {
      gearman_return_t ret;
      ret = gearman_worker_add_function(m_worker, function_name.c_str(), 0, function, 0);
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << gearman_worker_error(m_worker));
      }
    }

    void work() const {
      gearman_return_t ret;
      ret = gearman_worker_work(m_worker);
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << gearman_worker_error(m_worker));
      }
    }

    bool is_connected() const {
      return m_is_connected;
    }

    gearman_worker_st *worker() const {
      return m_worker;
    }

    ~GearmanWorkerWrapper() {
      gearman_worker_free(m_worker);
    }
};

class GearmanTaskList {
  gearman_client_st *m_client; 
  std::list<gearman_task_st *> m_tasks;

  std::list<gearman_task_st *> get_statuses() const {
    gearman_return_t ret;
    std::list<gearman_task_st *> statuses;

    BOOST_FOREACH(gearman_task_st *t, m_tasks) {
      statuses.push_back(gearman_client_add_task_status(m_client, 0, 0, gearman_task_job_handle(t), &ret));
      if (gearman_failed(ret) || !statuses.back()) {
        vw::vw_throw(vw::GearmanErr() << "Error adding status task");
      }
    }

    ret = gearman_client_run_tasks(m_client);
    if (gearman_failed(ret)) {
      vw::vw_throw(vw::GearmanErr() << "Error running status tasks");
    }

    VW_ASSERT(m_tasks.size() == statuses.size(), vw::LogicErr() << "Number of statuses doesn't match number of tasks");

    return statuses;
  }

  void prune() {
    std::list<gearman_task_st *> statuses = this->get_statuses();

    std::list<gearman_task_st *>::iterator task_iterator = m_tasks.begin();
    std::list<gearman_task_st *>::const_iterator status_iterator = statuses.begin();

    while (task_iterator != m_tasks.end()) {
      if (!gearman_task_is_known(*status_iterator)) {
        gearman_task_free(*task_iterator);
        task_iterator = m_tasks.erase(task_iterator);
      } else {
        task_iterator++;
      }
      gearman_task_free(*status_iterator++);
    }
  }

  public:
    GearmanTaskList(GearmanClientWrapper const& gclient)
      : m_client(gclient.client()), m_tasks() {}

    // TODO: generalize this to input a string instead of a job_request, and specify gearman
    // function name. this way the GearmanTaskList can be used for other things too
    void add_task(mvp::MVPJobRequest const& job_request, int curr_tile, int num_tiles) {
      gearman_return_t ret;

      // TODO: Need a better unique, otherwise will clash if multiple mvp clients are running...
      std::stringstream ss;
      ss << curr_tile;

      std::string message;
      job_request.SerializeToString(&message);

      m_tasks.push_back(gearman_client_add_task_background(m_client, 0, 0, "mvpalgorithm", ss.str().c_str(), message.c_str(), message.size(), &ret));
      if (gearman_failed(ret) || !m_tasks.back()) {
        vw::vw_throw(vw::GearmanErr() << "Error adding task");
      }

      ret = gearman_client_run_tasks(m_client);
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << "Error running tasks");
      }

      this->prune();
    }

    bool has_queued_tasks() const {
      bool ret = false;

      std::list<gearman_task_st *> statuses = this->get_statuses();
      BOOST_FOREACH(gearman_task_st *s, statuses) {
        if (gearman_task_is_known(s) && !gearman_task_is_running(s)) {
          ret = true;
        }
        gearman_task_free(s);
      }

      return ret;
    }

    bool has_running_tasks() const {
      bool ret = false;

      std::list<gearman_task_st *> statuses = this->get_statuses();
      BOOST_FOREACH(gearman_task_st *s, statuses) {
        if (gearman_task_is_known(s)) {
          ret = true;
        }
        gearman_task_free(s);
      }

      return ret;
    }

    void print_statuses() const {
      std::list<gearman_task_st *> statuses = this->get_statuses();

      static int last_print_size = 0;
      std::stringstream stream;

      std::list<gearman_task_st *>::const_iterator task_iterator = m_tasks.begin();
      std::list<gearman_task_st *>::const_iterator status_iterator = statuses.begin();
      while (task_iterator != m_tasks.end()) {
        gearman_task_st *t = *(task_iterator++);
        gearman_task_st *s = *(status_iterator++);

        if (gearman_task_denominator(s)) {
          double percent = 100 * gearman_task_numerator(s) / gearman_task_denominator(s);
          stream << "Task #" << gearman_task_unique(t) << ": [" << percent << "%] ";
        }

        gearman_task_free(s);
      }

      std::cout << std::setw(last_print_size) << std::left << stream.str() << "\r" << std::flush;
      last_print_size = stream.str().size();
    }
};

class GearmanProgressCallback : public vw::ProgressCallback {
  gearman_job_st *m_job;
  vw::TerminalProgressCallback m_pc;

  public:
    GearmanProgressCallback(gearman_job_st *job, std::string const& message)
      : m_job(job), m_pc("mvp", message) {}

    virtual void report_progress(double progress) const {
      //TODO: check abort
      gearman_return_t ret;
      ret = gearman_job_send_status(m_job, progress * 100, 100);
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << gearman_strerror(ret));
      }  
      m_pc.report_progress(progress);
    }

    virtual void report_incremental_progress(double incremental_progress) const {
      vw::vw_throw(vw::NoImplErr() << "Incremental progress not supported by GearmanProgressCallback");
    }

    virtual void report_finished() const {
      gearman_return_t ret;
      ret = gearman_job_send_status(m_job, 100, 100);
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << gearman_strerror(ret));
      }
      m_pc.report_finished();
    }
};

#endif
