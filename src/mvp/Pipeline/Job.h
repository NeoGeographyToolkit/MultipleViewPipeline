/// \file Job.h
///
/// Job class
///

#ifndef __MVP_PIPELINE_JOB_H__
#define __MVP_PIPELINE_JOB_H__

#include <mvp/Pipeline/JobDesc.pb.h>
#include <mvp/Image/OrbitalImageCollection.h>
#include <mvp/Algorithm/TileResult.h>

#include <vw/Cartography/GeoReference.h>

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

    image::OrbitalImageCollection orbital_images() const { return m_orbital_images; }

    AlgorithmSettings algorithm_settings() const { return m_job_desc.algorithm_settings(); }

    vw::cartography::GeoReference georef() const;

    vw::Vector2i tile_size() const;
};

}} // pipeline,mvp

#endif
