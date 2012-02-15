/// \file MVPJobBase.h
///
/// MVP Job Base class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOBBASE_H__
#define __MVP_MVPJOBBASE_H__

#include <vw/Plate/PlateGeoReference.h>
#include <vw/Cartography/GeoReference.h>
#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/Image/PixelMask.h>

#include <mvp/Config.h>
#include <mvp/MVPJobRequest.pb.h>
#include <mvp/MVPAlgorithmOptions.pb.h>
#include <mvp/OrbitalImageCrop.h>

#include <boost/foreach.hpp>

namespace mvp {

struct MVPAlgorithmVar {
  vw::float32 alt;

  vw::Vector3f orientation;
  vw::Vector3f windows;

  MVPAlgorithmVar(vw::float32 a = 0, vw::Vector3f const& o = vw::Vector3f(), vw::Vector3f const& w = vw::Vector3f()) :
    alt(a), orientation(o), windows(w) {}

  #if MVP_ENABLE_OCTAVE_SUPPORT
  MVPAlgorithmVar(::octave_scalar_map const& oct_map) : 
    alt(oct_map.getfield("alt").float_value()),
    orientation(vw::octave::octave_to_vector(oct_map.getfield("orientation").matrix_value())),
    windows(vw::octave::octave_to_vector(oct_map.getfield("windows").matrix_value())) {}

  ::octave_scalar_map to_octave() const {
    ::octave_scalar_map result;
    result.setfield("alt", alt);
    result.setfield("orientation", vw::octave::vector_to_octave(orientation));
    result.setfield("windows", vw::octave::vector_to_octave(windows));
    return result;
  }
  #endif
};

struct MVPPixelResult : public MVPAlgorithmVar {
  vw::float32 variance;
  bool converged;
  int num_iterations;

  MVPPixelResult(MVPAlgorithmVar const& mav = MVPAlgorithmVar(), vw::float32 v = 0, bool c = true, int n = 0) :
    MVPAlgorithmVar(mav), variance(v), converged(c), num_iterations(n) {}

  #if MVP_ENABLE_OCTAVE_SUPPORT
  MVPPixelResult(::octave_value_list oct_val_list) :
    MVPAlgorithmVar(oct_val_list(0).scalar_map_value()),
    variance(oct_val_list(1).float_value()),
    converged(oct_val_list(2).bool_value()),
    num_iterations(int(oct_val_list(3).double_value())) {}
  #endif
};


struct MVPTileResult {
  vw::cartography::GeoReference georef;

  vw::ImageView<vw::PixelMask<vw::float32> > alt;
  vw::ImageView<vw::PixelMask<vw::float32> > variance;

  vw::ImageView<vw::PixelMask<vw::Vector3f> > orientation;
  vw::ImageView<vw::PixelMask<vw::Vector3f> > windows;

  MVPTileResult(vw::cartography::GeoReference g, int tile_size) : georef(g),
    alt(tile_size, tile_size), variance(tile_size, tile_size), 
    orientation(tile_size, tile_size), windows(tile_size, tile_size) {}

  inline void update(int col, int row, MVPPixelResult const& px_result) {
    alt(col, row) = px_result.alt;
    variance(col, row) = px_result.variance;

    orientation(col, row) = px_result.orientation;
    windows(col, row) = px_result.windows;

    if (!px_result.converged) {
      alt(col, row).invalidate();
      variance(col, row).invalidate();
      orientation(col, row).invalidate();
      windows(col, row).invalidate();
    }
  }
};

template <class ImplT>
struct MVPJobBase {

  static ImplT construct_from_job_request(MVPJobRequest const& job_request) {
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

    return ImplT(georef, plate_georef.tile_size(), crops, job_request.user_settings());
  }

  inline ImplT& impl() {return static_cast<ImplT&>(*this);}
  inline ImplT const& impl() const {return static_cast<ImplT const&>(*this);}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const 
    {return impl().process_pixel(seed, georef, options);}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, double col, double row, MVPAlgorithmOptions const& options) const
    {return impl().process_pixel(seed, vw::cartography::crop(m_georef, col, row), options);}

  inline MVPPixelResult generate_seed() const {
    int seed_col = m_tile_size / 2;
    int seed_row = m_tile_size / 2;

    vw::Vector2 seed_lonlat = m_georef.pixel_to_lonlat(vw::Vector2(seed_col, seed_row));

    MVPAlgorithmVar pre_seed;
    pre_seed.alt = (m_settings.alt_min() + m_settings.alt_max()) / 2;
    VW_ASSERT(m_georef.datum().semi_major_axis() == m_georef.datum().semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 
    // TODO: The following calculation assumes spherical datum
    pre_seed.orientation = vw::cartography::lon_lat_radius_to_xyz(vw::Vector3(seed_lonlat[0], seed_lonlat[1], 1));
    pre_seed.windows = vw::Vector3(m_settings.seed_window_size(), m_settings.seed_window_size(), m_settings.seed_window_smooth_size());

    MVPAlgorithmOptions options;
    options.set_alt_range((m_settings.alt_max() - m_settings.alt_min()) / 2);
    options.set_fix_orientation(true);
    options.set_fix_windows(true);
    // TODO: set num_iterations?

    return process_pixel(pre_seed, seed_col, seed_row, options);
  }

  inline MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const {
    MVPTileResult tile_result(m_georef, m_tile_size);

    MVPPixelResult seed(generate_seed());

    if (!seed.converged) {
      return tile_result;
    }

    MVPAlgorithmOptions options;
    options.set_alt_range(m_settings.alt_search_range());
    // TODO: set num_iterations?

    // TODO: orientation and windows are fixed for testing.
    // window size is also explicitly set.
    options.set_fix_orientation(true); 
    options.set_fix_windows(true);
    seed.windows = vw::Vector3(10, 10, 1);

    int curr_px_num = 0;
    int num_px_to_process = m_tile_size * m_tile_size;
    for (int col = 0; col < m_tile_size; col++) {
      for (int row = 0; row < m_tile_size; row++) {
        progress.report_fractional_progress(curr_px_num++, num_px_to_process);
        tile_result.update(col, row, process_pixel(seed, col, row, options));
      }
    }
    progress.report_finished();
    return tile_result;
  }

  protected:
    vw::cartography::GeoReference m_georef;
    int m_tile_size;
    OrbitalImageCropCollection m_crops;
    MVPUserSettings m_settings;
    // TODO: A seed object...

    // This is defined here to prevent the user from accidently
    // constructing an MVPJobBase
    MVPJobBase(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
      m_georef(georef), m_tile_size(tile_size), m_crops(crops), m_settings(settings) {}
    MVPJobBase(MVPJobBase const& job) : m_georef(job.m_georef), m_tile_size(job.m_tile_size), m_crops(job.m_crops), m_settings(job.m_settings) {}

  private:
    // These are defined here to prevent them from being used
    MVPJobBase& operator=(MVPJobBase const&) {return *this;}
};

} // namespace mvp

#endif
