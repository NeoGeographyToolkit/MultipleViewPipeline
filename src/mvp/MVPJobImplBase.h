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
    result.setfield("alt", double(alt));
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

// TODO: Put this in the options
#define GAUSS_DIVISOR 6.0

class MVPJobImplBase {
  protected:
    vw::cartography::GeoReference m_georef;
    int m_tile_size;
    OrbitalImageCropCollection m_crops;
    MVPUserSettings m_settings;

  public:
    MVPJobImplBase(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
      m_georef(georef), m_tile_size(tile_size), m_crops(crops), m_settings(settings) {} 

    vw::cartography::GeoReference const& georef() const {return m_georef;}
    OrbitalImageCropCollection const& crops() const {return m_crops;}

    virtual MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const = 0;

    MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, double col, double row, MVPAlgorithmOptions const& options) const {
      return process_pixel(seed, vw::cartography::crop(m_georef, col, row), options);      
    }

    MVPPixelResult generate_seed() const {
      double seed_col = m_tile_size / 2.0;
      double seed_row = m_tile_size / 2.0;

      vw::Vector2 seed_lonlat = m_georef.pixel_to_lonlat(vw::Vector2(seed_col, seed_row));

      MVPAlgorithmVar pre_seed;
      pre_seed.alt = (m_settings.alt_min() + m_settings.alt_max()) / 2;
      VW_ASSERT(m_georef.datum().semi_major_axis() == m_georef.datum().semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 
      // TODO: The following calculation assumes spherical datum
      pre_seed.orientation = vw::cartography::lon_lat_radius_to_xyz(vw::Vector3(seed_lonlat[0], seed_lonlat[1], 1));
      pre_seed.windows = vw::Vector3(m_tile_size, m_tile_size, m_settings.seed_window_smooth_size());

      MVPAlgorithmOptions options;
      options.set_alt_range((m_settings.alt_max() - m_settings.alt_min()) / 2);
      options.set_fix_orientation(true);
      options.set_fix_windows(true);
      // TODO: set num_iterations?

      return process_pixel(pre_seed, seed_col, seed_row, options);
    }

    MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const {
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
};

} // namespace mvp

#endif
