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

} // namespace mvp

#endif
