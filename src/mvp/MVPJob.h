/// \file MVPJob.h
///
/// MVP Job class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOB_H__
#define __MVP_MVPJOB_H__

#include <mvp/MVPJobBase.h>
#include <boost/filesystem.hpp>
#include <vw/Image/MaskViews.h>
#include <vw/Plate/PlateFile.h>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <octave/parse.h>
#endif

namespace mvp {

struct MVPJob : public MVPJobBase<MVPJob> {

  MVPJob(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
    MVPJobBase<MVPJob>(georef, tile_size, crops, settings) {}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    using namespace vw;

    // TODO: MVP Algorithm implementation goes here...

    return MVPPixelResult();
  }
};

// TODO: Rename to MVPJobFootprint?
struct MVPJobTest : public MVPJobBase<MVPJobTest> {

  MVPJobTest(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
    MVPJobBase<MVPJobTest>(georef, tile_size, crops, settings) {}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    VW_ASSERT(georef.datum().semi_major_axis() == georef.datum().semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 
    using namespace vw;

    Vector2 ll = georef.pixel_to_lonlat(Vector2(0, 0));
    Vector3 llr(ll[0], ll[1], georef.datum().semi_major_axis());
    Vector3 xyz = cartography::lon_lat_radius_to_xyz(llr);

    int overlaps = 0;
    BOOST_FOREACH(OrbitalImageCrop const& o, m_crops) {
      Vector2 px = o.camera().point_to_pixel(xyz);
      if (bounding_box(o).contains(px)) {
        overlaps++;
      }
    }

    MVPAlgorithmVar result(overlaps, Vector3f(overlaps, overlaps, overlaps), Vector3f(overlaps, overlaps, overlaps));

    return MVPPixelResult(result, overlaps, overlaps > 0, overlaps);
  }
};

#if MVP_ENABLE_OCTAVE_SUPPORT
struct MVPJobOctave : public MVPJobBase<MVPJobOctave> {
  ::octave_map m_octave_crops;

  MVPJobOctave(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
    MVPJobBase<MVPJobOctave>(georef, tile_size, crops, settings) 
  {
    m_octave_crops = m_crops.to_octave();
  }

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    ::octave_value_list args;
    args.append(seed.to_octave());
    args.append(vw::octave::georef_to_octave(georef));
    args.append(m_octave_crops);
    args.append(vw::octave::protobuf_to_octave(&options));

    if (m_settings.test_algorithm()) {
      return MVPPixelResult(::feval(MVP_OCTAVE_FOOTPRINT_FCN, args, 1));
    } else {
      return MVPPixelResult(::feval(MVP_OCTAVE_ALGORITHM_FCN, args, 1));
    }
  }
};
#endif

MVPTileResult mvpjob_process_tile(MVPJobRequest const& job_request, 
                                  vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance())
{
  if (job_request.user_settings().use_octave()) {
    #if MVP_ENABLE_OCTAVE_SUPPORT
      return MVPJobOctave::construct_from_job_request(job_request).process_tile(progress);
    #else
      vw::vw_throw(vw::NoImplErr() << "Cannot use octave algorithm, as the MVP was not compled with it!");
    #endif
  } else if (job_request.user_settings().test_algorithm()) {
    return MVPJobTest::construct_from_job_request(job_request).process_tile(progress);
  } else {
    return MVPJob::construct_from_job_request(job_request).process_tile(progress);
  }
}

MVPTileResult mvpjob_process_and_write_tile(MVPJobRequest const& job_request, 
                                            vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance())
{
  MVPTileResult result = mvpjob_process_tile(job_request, progress);

  vw::ImageViewRef<vw::PixelGrayA<vw::float32> > rendered_tile = vw::mask_to_alpha(vw::pixel_cast<vw::PixelMask<vw::PixelGray<vw::float32> > >(result.alt));

  boost::scoped_ptr<vw::platefile::PlateFile> pf(new vw::platefile::PlateFile(job_request.result_platefile(),
                                                                              job_request.plate_georef().map_proj(),
                                                                              "MVP Result Plate",
                                                                              job_request.plate_georef().tile_size(),
                                                                              "tif", vw::VW_PIXEL_GRAYA, vw::VW_CHANNEL_FLOAT32));

  pf->transaction_begin("Post Heights", 1);
  pf->write_request();
  pf->write_update(rendered_tile, job_request.col(), job_request.row(), job_request.level());
  pf->sync();
  pf->write_complete();
  pf->transaction_end(true);

  return result;
}

mvp::MVPJobRequest load_job_file(std::string const& filename) {
  mvp::MVPJobRequest job_request;

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

MVPTileResult mvpjob_process_tile(std::string  const& job_filename, 
                                  vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) {
  return mvpjob_process_tile(load_job_file(job_filename), progress);
}

std::string save_job_file(MVPJobRequest const& job_request, std::string const& out_dir = ".") {
  namespace fs = boost::filesystem;

  // TODO: This is common code
  int col = job_request.col();
  int row = job_request.row();
  int level = job_request.level();

  vw::platefile::PlateGeoReference plate_georef(job_request.plate_georef());

  vw::cartography::GeoReference georef(plate_georef.tile_georef(col, row, level));

  vw::BBox2 tile_bbox(plate_georef.tile_lonlat_bbox(col, row, level));
  vw::Vector2 alt_limits(job_request.user_settings().alt_min(), job_request.user_settings().alt_max());

  OrbitalImageCropCollection crops(tile_bbox, georef.datum(), alt_limits);
  crops.add_image_collection(job_request.orbital_images());

  std::string job_filename;

  {
    std::stringstream stream;
    stream << out_dir << "/" << col << "_" << row << "_" << level << ".job";
    job_filename = stream.str();
  }

  // TODO: check IO errors
  fs::create_directory(job_filename);

  MVPJobRequest job_request_mod(job_request);
  for (unsigned curr_image = 0; curr_image < crops.size(); curr_image++) {
    std::stringstream stream;
    stream << curr_image;
    std::string str_num(stream.str());

    std::string image_name("image" + str_num + ".tif");
    std::string camera_name("camera" + str_num + ".pinhole");

    job_request_mod.mutable_orbital_images(curr_image)->set_camera_path(camera_name);
    job_request_mod.mutable_orbital_images(curr_image)->set_image_path(image_name);

    write_image(job_filename + "/" + image_name, crops[curr_image]);
    crops[curr_image].camera().write(job_filename + "/" + camera_name);
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

#endif
