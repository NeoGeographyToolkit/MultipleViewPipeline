/// \file MVPJob.h
///
/// MVP Job class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOB_H__
#define __MVP_MVPJOB_H__

#include <vw/Plate/PlateGeoReference.h>
#include <vw/Cartography/GeoReference.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

#include <mvp/MVPJobRequest.pb.h>
#include <mvp/OrbitalImageCrop.h>

#include <boost/foreach.hpp>

namespace mvp {

// TODO: This should be replaced by a function in VW
inline vw::cartography::GeoReference offset_georef(vw::cartography::GeoReference const& georef, int cols, int rows) {
  vw::Matrix3x3 offset;

  offset.set_identity();
  offset(0, 2) = cols;
  offset(1, 2) = rows;

  vw::cartography::GeoReference result(georef);
  // TODO: This is slow...
  result.set_transform(georef.transform() * offset);
  return result; 
}


struct MVPAlgorithmVar {
  vw::float32 post_height;

  vw::Vector3f orientation;
  vw::Vector3f windows;

  MVPAlgorithmVar(vw::float32 h = 0, vw::Vector3f const& o = vw::Vector3f(), vw::Vector3f const& w = vw::Vector3f()) :
    post_height(h), orientation(o), windows(w) {}
  
  // to_octave()
};

struct MVPPixelResult : public MVPAlgorithmVar {
  vw::float32 variance;
  bool converged;
  int num_iterations_to_converge;

  MVPPixelResult(MVPAlgorithmVar const& mav = MVPAlgorithmVar(), vw::float32 v = 0, bool c = true, int n = 0) :
    MVPAlgorithmVar(mav), variance(v), converged(c), num_iterations_to_converge(n) {}

  // constructor from octave vars
};


struct MVPTileResult {
  vw::cartography::GeoReference georef;

  vw::ImageView<vw::float32> post_height;
  vw::ImageView<vw::float32> variance;

  vw::ImageView<vw::Vector3f> orientation;
  vw::ImageView<vw::Vector3f> windows;

  MVPTileResult(vw::cartography::GeoReference g, int tile_size) : georef(g),
    post_height(tile_size, tile_size), variance(tile_size, tile_size), 
    orientation(tile_size, tile_size), windows(tile_size, tile_size) {}

  inline void update(int col, int row, MVPPixelResult const& px_result) {
    post_height(col, row) = px_result.post_height;
    variance(col, row) = px_result.variance;

    orientation(col, row) = px_result.orientation;
    windows(col, row) = px_result.windows;
  }
};

template <class ImplT>
struct MVPJobBase {
  inline ImplT& impl() {return static_cast<ImplT&>(*this);}
  inline ImplT const& impl() const {return static_cast<ImplT const&>(*this);}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const 
    {return impl().process_pixel(seed, georef);}

  inline MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) {
    MVPTileResult tile_result(m_georef, m_tile_size);
    MVPAlgorithmVar seed;

    int curr_px_num = 0;
    int num_px_to_process = m_tile_size * m_tile_size;
    progress.report_progress(0);
    for (int col = 0; col < m_tile_size; col++) {
      for (int row = 0; row < m_tile_size; row++) {
        tile_result.update(col, row, process_pixel(seed, offset_georef(m_georef, col, row)));
        progress.report_progress(double(++curr_px_num) / num_px_to_process);
      }
    }
    progress.report_finished();
    return tile_result;
  }

  protected:
    MVPAlgorithmSettings m_settings;
    vw::cartography::GeoReference m_georef;
    int m_tile_size;
    OrbitalImageCropCollection m_crops;
    // TODO: A seed object...

    // This is defined here to prevent the user from accidently
    // constructing an MVPJobBase
    MVPJobBase(MVPJobRequest job_request) : 
      m_settings(job_request.algorithm_settings()),
      m_georef(job_request.georef()),
      m_tile_size(job_request.tile_size()),
      m_crops(job_request.orbital_images(), m_georef.pixel_to_lonlat_bbox(vw::BBox2(0, 0, m_tile_size, m_tile_size))) {}

  private:
    // These are defined here to prevent them from being used
    MVPJobBase(MVPJobBase const&) {}
    MVPJobBase& operator=(MVPJobBase const&) {return *this;}
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
