#ifndef __MVP_GEARMAN_WRAPPERS_H__
#define __MVP_GEARMAN_WRAPPERS_H__

#include <mvp/MVPJobRequest.pb.h>
#include <libgearman/gearman.h>

namespace vw {
  VW_DEFINE_EXCEPTION(GearmanErr, Exception);
}

class GearmanClientWrapper {
  gearman_client_st *m_client;
  bool m_has_servers;

  public:
    GearmanClientWrapper() : m_has_servers(false) {
      m_client = gearman_client_create(NULL);
      if (!m_client) {
        throw std::bad_alloc();
      }
    }

    GearmanClientWrapper(GearmanClientWrapper const& other)
      : m_has_servers(other.m_has_servers) {
      m_client = gearman_client_clone(NULL, other.m_client);
      if (!m_client) {
        throw std::bad_alloc();
      }
    }

    void add_servers(std::string const& servers) {
      gearman_return_t ret;
      ret = gearman_client_add_servers(m_client, servers.c_str());
      if (gearman_failed(ret)) {
        vw::vw_throw(vw::GearmanErr() << gearman_client_error(m_client));
      }
      m_has_servers = true;
    }

    bool has_servers() const {
      return m_has_servers;
    }

    gearman_client_st *client() const {
      return m_client;
    }
  
    ~GearmanClientWrapper() {
      gearman_client_free(m_client);
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

    void wait_until_everything_is_running() const {
      bool waiting;

      do {
        this->print_statuses();

        waiting = false;
        std::list<gearman_task_st *> statuses = this->get_statuses();
        BOOST_FOREACH(gearman_task_st *s, statuses) {
          if (gearman_task_is_known(s) && !gearman_task_is_running(s)) {
            waiting = true;
          }
          gearman_task_free(s);
        }

        if (waiting) {
          sleep(1);
        }
      } while (waiting);
    }

    void wait_until_everything_is_done() const {
      bool waiting;

      do {
        this->print_statuses();
       
        waiting = false; 
        std::list<gearman_task_st *> statuses = this->get_statuses();
        BOOST_FOREACH(gearman_task_st *s, statuses) {
          if (gearman_task_is_known(s)) {
            waiting = true;
          }
          gearman_task_free(s);
        }

        if (waiting) {
          sleep(1);
        }
      } while (waiting);
    }
};

#endif
