#include <mvp/Pipeline/Job.h>

#include <mvp/Algorithm/PixelResult.h>
#include <mvp/Algorithm/Lighter.h>
#include <mvp/Algorithm/Stepper.h>
#include <mvp/Algorithm/Seeder.h>
#include <mvp/Algorithm/Objective.h>
#include <mvp/Algorithm/Correlator.h>

#include <vw/Plate/PlateGeoReference.h>
#include <vw/Plate/PlateFile.h>
#include <vw/Camera/PinholeModel.h>

#include <boost/filesystem.hpp>

#include <unistd.h> // usleep

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <mvp/Octave/Conversions.h>

#include <octave/symtab.h>
#include <octave/load-save.h>
#endif


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

  Objective objective(m_job_desc.algorithm_settings().objective_settings());
  Lighter lighter(m_job_desc.algorithm_settings().lighter_settings());

  Correlator correlator0(m_orbital_images, lighter, objective,
                         m_job_desc.algorithm_settings().correlator0_settings());

  Correlator correlator(m_orbital_images, lighter, objective,
                        m_job_desc.algorithm_settings().correlator_settings());

  Seeder seeder(georef(), tile_size(), m_job_desc.algorithm_settings().seeder_settings());


  while (!seeder.done()) {
    PixelResult result = correlator.correlate(seeder.curr_post(), seeder.curr_seed());
    seeder.update(result);
  }

  std::cout << "Seeder result: " << seeder.result()[0].value().algorithm_var().radius() -
                                    georef().datum().semi_major_axis() << std::endl;

  Stepper stepper(georef(), tile_size(), seeder.result(), m_job_desc.algorithm_settings().stepper_settings());

  int cursor = 1;
  int total = tile_size()[0] * tile_size()[1];

  while (!stepper.done()) {
    progress.report_fractional_progress(cursor, total);
    PixelResult result = correlator.correlate(stepper.curr_post(), stepper.curr_seed());
    stepper.update(result);
    cursor += 1;

    // TODO: make an option in global settings
    if (cursor % 100) {
      update_platefile(stepper.result());
    }
  }

  progress.report_finished();

  return stepper.result();
}

void Job::update_platefile(algorithm::TileResult const& result) const {
  using namespace vw;

  boost::scoped_ptr<platefile::PlateFile> pf(new platefile::PlateFile(m_job_desc.output().platefile()));

  pf->transaction_begin("", 1);
  pf->write_request();
  ImageView<PixelGrayA<float32> > raster = pixel_cast<PixelGrayA<float32> >(result.plate_layer(1));
  pf->write_update(raster, m_job_desc.render().col(), 
                           m_job_desc.render().row(), 
                           m_job_desc.render().level());
  pf->sync();
  pf->write_complete();
  pf->transaction_end(true);
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

#if MVP_ENABLE_OCTAVE_SUPPORT

std::string Job::save_job_file_octave(std::string const& out_dir) const {
  namespace fs = boost::filesystem;

  std::string job_filename;

  {
    std::stringstream stream;
    stream << out_dir << "/" << m_job_desc.render().col() << "_" << m_job_desc.render().row()<< "_" << m_job_desc.render().level() << ".job.mat";
    job_filename = stream.str();
  }

  octave_scalar_map oct_datum;
  oct_datum.setfield("semi_major_axis", georef().datum().semi_major_axis());
  oct_datum.setfield("semi_minor_axis", georef().datum().semi_major_axis());

  octave_scalar_map oct_georef;
  oct_georef.setfield("datum", oct_datum);
  oct_georef.setfield("transform", octave::to_octave(georef().transform()));

  octave_value_list oct_images;
  octave_value_list oct_cameras;
  BOOST_FOREACH(image::OrbitalImage const& o, m_orbital_images) {
    oct_images.append(octave::to_octave(o.image()));
    oct_cameras.append(octave::to_octave(o.camera().to_pinhole().camera_matrix()));
  }

  octave_map oct_orbital_images;
  oct_orbital_images.setfield("data", oct_images);
  oct_orbital_images.setfield("camera", oct_cameras);

  octave_scalar_map result;
  result.setfield("georef", oct_georef);
  result.setfield("algorithm_settings", octave::to_octave(m_job_desc.algorithm_settings()));
  result.setfield("orbital_images", oct_orbital_images);
  result.setfield("tile_size", octave::to_octave(tile_size()));

  std::ofstream os(job_filename.c_str(), std::ios::binary);

  symbol_table::symbol_record varname("job", result);
  load_save_format format = LS_MAT5_BINARY;
  bool save_as_floats = false;

  write_header(os, format);
  do_save(os, varname, format, save_as_floats);

  return job_filename;
}

#endif

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
