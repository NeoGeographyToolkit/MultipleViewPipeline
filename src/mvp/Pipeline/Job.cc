#include <mvp/Pipeline/Job.h>

#include <unistd.h> // usleep

namespace mvp {
namespace pipeline {

Job::Job(std::string const& job_file) {
  vw::vw_throw(vw::NoImplErr() << "Not implemented!");
}

algorithm::TileResult Job::process_tile(vw::ProgressCallback const& progress) const {
  using namespace algorithm;

  TileResult result(m_job_desc.output().result(), 
                    m_job_desc.render().col(), 
                    m_job_desc.render().row(), 
                    m_job_desc.render().level(),
                    m_job_desc.output().plate_georef().tile_size()); 

  for (int i = 0; i < 100; i++) {
    usleep(1000000);
    progress.report_fractional_progress(i, 100);
  }

  progress.report_finished();

  return result;
}

std::string Job::save_job_file(std::string const& out_dir) const {
  return std::string();
}

}} // namespace pipeline,mvp
