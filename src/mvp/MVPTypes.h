/// \file MVPTypes.h
///
/// MVP Types
///
/// TODO: Write something here
///

#ifndef __MVP_MVPTYPES_H__
#define __MVP_MVPTYPES_H__

#include <mvp/Config.h>
#include <vw/Cartography/GeoReference.h>
#include <vw/Image/PixelMask.h>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <vw/Octave/Conversions.h>
#endif

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
  int tile_size;

  vw::ImageView<vw::PixelMask<vw::float32> > alt;
  vw::ImageView<vw::PixelMask<vw::float32> > variance;

  vw::ImageView<vw::PixelMask<vw::Vector3f> > orientation;
  vw::ImageView<vw::PixelMask<vw::Vector3f> > windows;

  MVPTileResult(vw::cartography::GeoReference g, int t) : georef(g),
    tile_size(t), alt(t, t), variance(t, t), 
    orientation(t, t), windows(t, t) {}

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

} // namespace mvp

#endif
