/// \file MVPJob.h
///
/// MVP Job class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOB_H__
#define __MVP_MVPJOB_H__

#include <mvp/MVPJobBase.h>

namespace mvp {

struct MVPJob : public MVPJobBase<MVPJob> {
  
  MVPJob(MVPJobRequest const& job_request) : MVPJobBase(job_request) {}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const {
    using namespace vw;

    // TODO: MVP Algorithm implementation goes here...

    return MVPPixelResult();
  }
};

struct MVPJobTest : public MVPJobBase<MVPJobTest> {

  MVPJobTest(MVPJobRequest const& job_request) : MVPJobBase(job_request) {}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const {
    using namespace vw;

    Vector2 ll = georef.pixel_to_lonlat(Vector2(0, 0));
    Vector3 llr(ll[0], ll[1], georef.datum().radius(ll[0], ll[1]));
    Vector3 xyz = cartography::lon_lat_radius_to_xyz(llr);

    int overlaps = 0;
    BOOST_FOREACH(OrbitalImageCrop const& o, m_crops) {
      Vector2 px = o.camera().point_to_pixel(xyz);
      if (bounding_box(o.image()).contains(px)) {
        overlaps++;
      }
    }

    MVPAlgorithmVar result(overlaps, Vector3f(overlaps, overlaps, overlaps), Vector3f(overlaps, overlaps, overlaps));

    return MVPPixelResult(result, overlaps, true, overlaps);
  }
};

inline MVPTileResult mvpjob_process_tile(MVPJobRequest const& job_request, vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) {
  if (job_request.algorithm_settings().use_octave()) {
    #if MVP_ENABLE_OCTAVE_SUPPORT
      return MVPJobOctave(job_request).process_tile(progress);
    #else
      vw::vw_throw(vw::NoImplErr() << "Cannot use octave algorithm, as the MVP was not compled with it!");
    #endif
  } else if (job_request.algorithm_settings().test_algorithm()) {
    return MVPJobTest(job_request).process_tile(progress);
  } else {
    return MVPJob(job_request).process_tile(progress);
  }
}

} // namespace mvp

#endif