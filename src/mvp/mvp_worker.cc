#include <libgearman/gearman.h>

#include <iostream>

#include <boost/program_options.hpp>

#include <mvp/MVPJob.h>
#include <vw/Octave/Main.h>

#if MVP_ENABLE_GEARMAN_SUPPORT
#include <mvp/GearmanWrappers.h>
#endif

using namespace std;
using namespace mvp;
using namespace vw;
namespace po = boost::program_options;

struct Options {
  po::variables_map vm;
  string gearman_server;
  string job_filename;
};

void handle_arguments(int argc, char* argv[], Options *opts) {
  po::options_description cmd_opts("Options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("gearman-server", po::value<string>(&opts->gearman_server)->default_value("localhost"), "Host running gearmand")
    ("job,j", po::value<string>(&opts->job_filename), "Process a jobfile")
    ;

  store(po::command_line_parser(argc, argv).options(cmd_opts).run(), opts->vm);
  
  if (opts->vm.count("help")) {
    vw_throw(vw::ArgumentErr() << cmd_opts);
  }

  notify(opts->vm);
}

#if MVP_ENABLE_GEARMAN_SUPPORT
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
#endif

int main(int argc, char *argv[]) {
  #if MVP_ENABLE_OCTAVE_SUPPORT
  vw::octave::start_octave_interpreter();
  #endif

  Options opts;

  try {
    handle_arguments(argc, argv, &opts);
  } catch (const vw::ArgumentErr& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  gearman_return_t ret;
  gearman_worker_st *worker;

  worker = gearman_worker_create(NULL);

  if (!worker) {
    cout << "Error creating gearman worker" << endl;
    return 1;
  }

  ret = gearman_worker_add_servers(worker, opts.gearman_server.c_str());
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
