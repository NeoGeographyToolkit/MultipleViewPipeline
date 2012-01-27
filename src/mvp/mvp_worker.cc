#include <libgearman/gearman.h>

#include <iostream>

#include <boost/program_options.hpp>

#include <mvp/MVPJob.h>
#include <vw/Octave/Main.h>

using namespace std;
using namespace mvp;
namespace po = boost::program_options;

class GearmanProgressCallback : public vw::ProgressCallback {
  gearman_job_st *m_job;
  vw::TerminalProgressCallback m_pc;

  public:
    GearmanProgressCallback(gearman_job_st *job, string const& message)
      : m_job(job), m_pc("mvp", message) {}

    virtual void report_progress(double progress) const {
      m_pc.report_progress(progress);
      //TODO: throw on error?
      gearman_job_send_status(m_job, progress * 100, 100);
    }

    virtual void report_incremental_progress(double incremental_progress) const {
      vw_throw(vw::NoImplErr() << "Incremental progress not supported by GearmanProgressCallback");
    }

    virtual void report_finished() const {
      m_pc.report_finished();
      //TODO: throw on error?
      gearman_job_send_status(m_job, 100, 100);
    }
};

static void *mvpalgorithm_gearman(gearman_job_st *job, void *context, 
                                  size_t *result_size, gearman_return_t *ret_ptr) 
{
  MVPJobRequest job_request;
  job_request.ParseFromString(string((const char *)gearman_job_workload(job), gearman_job_workload_size(job)));

  stringstream ss;
  ss << "[" << job_request.col() << ", " << job_request.row() << "] @ " << job_request.level() << ": ";

  GearmanProgressCallback progress(job, ss.str());

  mvpjob_process_and_write_tile(job_request, progress);

  *ret_ptr = GEARMAN_SUCCESS;
  *result_size = 0;
  return NULL;
}


int main(int argc, char *argv[]) {
  #if MVP_ENABLE_OCTAVE_SUPPORT
  vw::octave::start_octave_interpreter();
  #endif

  po::options_description cmd_opts("Options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("gearman-servers", po::value<string>()->default_value("localhost"), "Host of gearmand")
    ;

  po::variables_map vm;
  store(po::command_line_parser(argc, argv).options(cmd_opts).run(), vm);
  
  if (vm.count("help")) {
    cout << cmd_opts << endl;
    return 0;
  }

  notify(vm);

  gearman_return_t ret;
  gearman_worker_st *worker;

  worker = gearman_worker_create(NULL);

  if (!worker) {
    cout << "Error creating gearman worker" << endl;
    return 1;
  }

  ret = gearman_worker_add_servers(worker, vm["gearman-servers"].as<string>().c_str());
  if (gearman_failed(ret)) {
    cout << "Gearman error: " << gearman_worker_error(worker) << endl;
    return 1;
  }

  ret = gearman_worker_add_function(worker, "mvpalgorithm", 0, mvpalgorithm_gearman, 0);
  if (gearman_failed(ret)) {
    cout << "Gearman error: " << gearman_worker_error(worker) << endl;
    return 1;
  }

  while (1) {
    ret = gearman_worker_work(worker);
    if (gearman_failed(ret)) {
      std::cout << "Gearman error: " << gearman_worker_error(worker) << endl;
      break;
    }
  }

  gearman_worker_free(worker);

  #if MVP_ENABLE_OCTAVE_SUPPORT
  do_octave_atexit();
  #endif

  return 0;



}
