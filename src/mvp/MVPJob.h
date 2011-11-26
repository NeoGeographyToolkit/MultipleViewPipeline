/// \file MVPJob.h
///
/// MVP Job class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOB_H__
#define __MVP_MVPJOB_H__

#include <mvp/MVPJobBase.h>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <octave/parse.h>
#include <octave/octave.h>
#include <octave/load-path.h>
#include <octave/toplev.h>
#endif

namespace mvp {

struct MVPJob : public MVPJobBase<MVPJob> {

  MVPJob(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPAlgorithmSettings const& settings) :
    MVPJobBase<MVPJob>(georef, tile_size, crops, settings) {}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const {
    using namespace vw;

    // TODO: MVP Algorithm implementation goes here...

    return MVPPixelResult();
  }
};

struct MVPJobTest : public MVPJobBase<MVPJobTest> {

  MVPJobTest(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPAlgorithmSettings const& settings) :
    MVPJobBase<MVPJobTest>(georef, tile_size, crops, settings) {}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const {
    using namespace vw;

    Vector2 ll = georef.pixel_to_lonlat(Vector2(0, 0));
    Vector3 llr(ll[0], ll[1], georef.datum().radius(ll[0], ll[1]));
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
  ::octave_scalar_map m_octave_settings;

  MVPJobOctave(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPAlgorithmSettings const& settings) :
    MVPJobBase<MVPJobOctave>(georef, tile_size, crops, settings) 
  {
    m_octave_crops = m_crops.to_octave();
    m_octave_settings = vw::octave::protobuf_to_octave(&settings); 
  }

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const {
    ::octave_value_list args;
    args.append(seed.to_octave());
    args.append(vw::octave::georef_to_octave(georef));
    args.append(m_octave_crops);
    args.append(m_octave_settings);

    return MVPPixelResult(::feval(MVP_OCTAVE_ALGORITHM_FCN, args, 1));
  }

  static void start_interpreter(std::string const& mfile_dir = MVP_MFILE_INSTALL_DIR, std::string const& octfile_dir = MVP_OCTFILE_INSTALL_DIR) {
    const char * argvv [] = {"", "--silent"};
    ::octave_main (2, (char **) argvv, true);
    ::load_path::append(mfile_dir);
    ::load_path::append(octfile_dir);
  }
};
#endif

inline MVPTileResult mvpjob_process_tile(MVPJobRequest const& job_request, vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) {
  if (job_request.algorithm_settings().use_octave()) {
    #if MVP_ENABLE_OCTAVE_SUPPORT
      return MVPJobOctave::construct_from_job_request(job_request).process_tile(progress);
    #else
      vw::vw_throw(vw::NoImplErr() << "Cannot use octave algorithm, as the MVP was not compled with it!");
    #endif
  } else if (job_request.algorithm_settings().test_algorithm()) {
    return MVPJobTest::construct_from_job_request(job_request).process_tile(progress);
  } else {
    return MVPJob::construct_from_job_request(job_request).process_tile(progress);
  }
}

} // namespace mvp

#endif
