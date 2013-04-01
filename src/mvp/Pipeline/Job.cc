#include <mvp/Pipeline/Job.h>

#include <vw/Plate/PlateGeoReference.h>

#include <boost/filesystem.hpp>

#include <unistd.h> // usleep

namespace mvp {
namespace pipeline {

Job::Job(std::string const& job_file) {
  JobDesc job_desc;

  std::fstream input((job_file + "/job").c_str(), std::ios::in | std::ios::binary);
  if (!input) {
    vw_throw(vw::IOErr() << "Missing: /job");
  } else if (!job_desc.ParseFromIstream(&input)) {
    vw_throw(vw::IOErr() << "Unable to process /job");
  }

  BOOST_FOREACH(image::OrbitalImageDesc& o, *job_desc.mutable_input()->mutable_orbital_images()) {
    o.set_image_path(job_file + "/" + o.image_path());
    o.set_camera_path(job_file + "/" + o.camera_path());
  }

  m_job_desc = job_desc;
  m_orbital_images.push_back_container(job_desc.input().orbital_images());
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
  namespace fs = boost::filesystem;

  std::string job_filename;

  {
    std::stringstream stream;
    stream << out_dir << "/" << m_job_desc.render().col() << "_" << m_job_desc.render().row()<< "_" << m_job_desc.render().level() << ".job";
    job_filename = stream.str();
  }

  // TODO: check IO errors
  fs::create_directory(job_filename);

  JobDesc job_desc_mod(m_job_desc);
  job_desc_mod.mutable_input()->clear_orbital_images();

  std::vector<image::OrbitalImageDesc> saved_orbital_images;
  for (unsigned i = 0; i < m_orbital_images.size(); i++) {
    std::stringstream ss;
    ss << job_filename << "/image" << i;
    saved_orbital_images.push_back(m_orbital_images[i].write(ss.str()));
  }

  std::copy(saved_orbital_images.begin(), saved_orbital_images.end(), RepeatedFieldBackInserter(job_desc_mod.mutable_input()->mutable_orbital_images()));

  // Use relative paths
  BOOST_FOREACH(image::OrbitalImageDesc &o, *job_desc_mod.mutable_input()->mutable_orbital_images()) {
    o.set_image_path(fs::path(o.image_path()).filename().string());
    o.set_camera_path(fs::path(o.camera_path()).filename().string());
  }

  {
    std::fstream output((job_filename + "/job").c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!job_desc_mod.SerializeToOstream(&output)) {
      vw_throw(vw::IOErr() << "Failed to serialize jobfile");
    }
  }

  return job_filename;
}

vw::cartography::GeoReference Job::georef() const {
  vw::platefile::PlateGeoReference plate_georef(m_job_desc.output().plate_georef());
  return plate_georef.tile_georef(m_job_desc.render().col(), 
                                  m_job_desc.render().row(),
                                  m_job_desc.render().level());
}

vw::Vector2i Job::tile_size() const {
  vw::platefile::PlateGeoReference plate_georef(m_job_desc.output().plate_georef());
  return vw::Vector2i(plate_georef.tile_size(), plate_georef.tile_size());
}

}} // namespace pipeline,mvp
