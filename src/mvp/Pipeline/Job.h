/// \file Job.h
///
/// Job class
///

#ifndef __MVP_PIPELINE_JOB_H__
#define __MVP_PIPELINE_JOB_H__

#include <mvp/Config.h>

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
    void update_platefile(algorithm::TileResult const& result) const;

    std::string save_job_file(std::string const& out_dir = ".") const;

#if MVP_ENABLE_OCTAVE_SUPPORT    
    std::string save_job_file_octave(std::string const& out_dir = ".") const;
#endif

    image::OrbitalImageCollection orbital_images() const { return m_orbital_images; }

    AlgorithmSettings algorithm_settings() const { return m_job_desc.algorithm_settings(); }

    vw::cartography::GeoReference georef() const;

    vw::Vector2i tile_size() const;

    int plate_col() const { return m_job_desc.render().col(); }

    int plate_row() const { return m_job_desc.render().row(); }

    int plate_level() const { return m_job_desc.render().level(); }
};

}} // pipeline,mvp

#endif
