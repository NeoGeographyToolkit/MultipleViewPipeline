/// \file Job.h
///
/// Job class
///

#ifndef __MVP_PIPELINE_JOB_H__
#define __MVP_PIPELINE_JOB_H__

#include <mvp/Pipeline/JobDesc.pb.h>
#include <mvp/Algorithm/Types.h>
#include <mvp/Image/OrbitalImageCollection.h>

namespace mvp {
namespace pipeline {

class Job {
  JobDesc m_job_desc;
  image::OrbitalImageCollection m_orbital_images;

  public:
    Job(JobDesc const& job_desc) : m_job_desc(job_desc), m_orbital_images(job_desc.input().orbital_images()) {}
    
    Job(std::string const& job_file);

    algorithm::TileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const;

    std::string save_job_file(std::string const& out_dir = ".") const;
};

}} // pipeline,mvp

#endif
