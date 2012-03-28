#include <mvp/MVPJob.h>
#include <mvp/MVPAlgorithm.h>
#include <mvp/MVPTileSeeder.h>
#include <mvp/MVPTileProcessor.h>

#include <vw/Plate/PlateGeoReference.h> // TODO: get rid of me
#include <vw/Plate/PlateFile.h>
#include <vw/Image/ImageViewRef.h>
#include <vw/Image/MaskViews.h>

#include <boost/filesystem.hpp>

namespace mvp {

MVPJob::MVPJob(MVPJobRequest const& job_request) : 
  m_job_request(job_request) {

  // TODO: Make MVPJobRequest send a GeoRef and a tile_lonlat_bbox to avoid these calcs 
  int col = job_request.col();
  int row = job_request.row();
  int level = job_request.level();
  vw::platefile::PlateGeoReference plate_georef(m_job_request.plate_georef());
  vw::cartography::GeoReference georef = plate_georef.tile_georef(col, row, level);
  OrbitalImageCropCollection crops(plate_georef.tile_lonlat_bbox(col, row, level),
                                   plate_georef.datum(),
                                   m_job_request.user_settings().alt_min(),
                                   m_job_request.user_settings().alt_max());
  crops.add_image_collection(m_job_request.orbital_images());
  m_crops = crops;
}

MVPTileResult MVPJob::process_tile(vw::ProgressCallback const& progress) const {
  boost::scoped_ptr<MVPAlgorithm> algorithm;

  if (m_job_request.use_octave()) {
    #if MVP_ENABLE_OCTAVE_SUPPORT
      if (m_job_request.draw_footprints()) {
        algorithm.reset(new MVPAlgoOctave(m_crops, MVP_OCTAVE_FOOTPRINT_FCN));
      } else {
        algorithm.reset(new MVPAlgoOctave(m_crops, MVP_OCTAVE_ALGORITHM_FCN));
      }
    #else
      vw::vw_throw(vw::NoImplErr() << "Cannot use octave algorithm, as the MVP was not compiled with it!");
    #endif
  } else {
    if (m_job_request.draw_footprints()) {
      algorithm.reset(new MVPAlgoFootprint(m_crops));
    } else {
      algorithm.reset(new MVPAlgoImpl(m_crops));
    }
  }

  // TODO: Make MVPJobRequest send a GeoRef and a tile_lonlat_bbox to avoid these calcs 
  int col = m_job_request.col();
  int row = m_job_request.row();
  int level = m_job_request.level();
  vw::platefile::PlateGeoReference plate_georef(m_job_request.plate_georef());
  vw::cartography::GeoReference georef = plate_georef.tile_georef(col, row, level);

  MVPTileSeederDumb seeder(algorithm.get(), georef, plate_georef.tile_size(), m_job_request.user_settings());
  MVPTileProcessorDumb processor(&seeder);

  return processor(progress);
}

void MVPJob::write_tile(MVPTileResult const& result) const {
  int col = m_job_request.col();
  int row = m_job_request.row();
  int level = m_job_request.level();

  vw::ImageViewRef<vw::PixelGrayA<vw::float32> > rendered_tile = vw::mask_to_alpha(vw::pixel_cast<vw::PixelMask<vw::PixelGray<vw::float32> > >(result.alt));

  boost::scoped_ptr<vw::platefile::PlateFile> pf(new vw::platefile::PlateFile(m_job_request.result_platefile()));

  pf->transaction_begin("Post Heights", 1);
  pf->write_request();
  pf->write_update(rendered_tile, col, row, level);
  pf->sync();
  pf->write_complete();
  pf->transaction_end(true);

  // TODO: Write the other tiles
}

MVPTileResult MVPJob::process_and_write_tile(vw::ProgressCallback const& progress) const {
  MVPTileResult result(process_tile(progress));
  write_tile(result);
  return result;
}

MVPJobRequest MVPJob::load_job_file(std::string const& filename) {
  MVPJobRequest job_request;

  std::fstream input((filename + "/job").c_str(), std::ios::in | std::ios::binary);      
  if (!input) {
    vw_throw(vw::IOErr() << "Missing: /job");
  } else if (!job_request.ParseFromIstream(&input)) {
    vw_throw(vw::IOErr() << "Unable to process /job");
  }

  BOOST_FOREACH(mvp::OrbitalImageFileDescriptor& o, *job_request.mutable_orbital_images()) {
    o.set_image_path(filename + "/" + o.image_path());
    o.set_camera_path(filename + "/" + o.camera_path());
  }

  return job_request;
}

std::string MVPJob::save_job_file(std::string const& out_dir) const {
  namespace fs = boost::filesystem;

  std::string job_filename;

  {
    std::stringstream stream;
    stream << out_dir << "/" << m_job_request.col() << "_" << m_job_request.row()<< "_" << m_job_request.level() << ".job";
    job_filename = stream.str();
  }

  // TODO: check IO errors
  fs::create_directory(job_filename);

  MVPJobRequest job_request_mod(m_job_request);
  for (unsigned curr_image = 0; curr_image < m_crops.size(); curr_image++) {
    std::stringstream stream;
    stream << curr_image;
    std::string str_num(stream.str());

    std::string image_name("image" + str_num + ".tif");
    std::string camera_name("camera" + str_num + ".pinhole");

    job_request_mod.mutable_orbital_images(curr_image)->set_camera_path(camera_name);
    job_request_mod.mutable_orbital_images(curr_image)->set_image_path(image_name);

    vw::write_image(job_filename + "/" + image_name, m_crops[curr_image]);
    m_crops[curr_image].camera().write(job_filename + "/" + camera_name);
  }

  {
    std::fstream output((job_filename + "/job").c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
    if (!job_request_mod.SerializeToOstream(&output)) {
      vw_throw(vw::IOErr() << "Failed to serialize jobfile");
    }
  }

  return job_filename;
}

} // namespace mvp
