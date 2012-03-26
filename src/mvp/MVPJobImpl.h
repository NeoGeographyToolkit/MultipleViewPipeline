/// \file MVPJobImpl.h
///
/// MVP Job Impl classes
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOBIMPL_H__
#define __MVP_MVPJOBIMPL_H__

#include <mvp/MVPJobImplBase.h>

namespace mvp {

struct MVPJobImpl : public MVPJobImplBase {

  MVPJobImpl(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
    MVPJobImplBase(georef, tile_size, crops, settings) {}

  virtual MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const;
};

struct MVPJobImplFootprint : public MVPJobImplBase {

  MVPJobImplFootprint(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
    MVPJobImplBase(georef, tile_size, crops, settings) {}

  virtual MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const;
};

#if MVP_ENABLE_OCTAVE_SUPPORT
struct MVPJobImplOctave : public MVPJobImplBase {
  ::octave_map m_octave_crops;

  MVPJobImplOctave(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
    MVPJobImplBase(georef, tile_size, crops, settings) {
    m_octave_crops = m_crops.to_octave();
  }

  virtual MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const;
};

struct MVPJobImplFootprintOctave : public MVPJobImplBase {
  ::octave_map m_octave_crops;

  MVPJobImplFootprintOctave(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
    MVPJobImplBase(georef, tile_size, crops, settings) {
    m_octave_crops = m_crops.to_octave();
  }

  virtual MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const; 
};
#endif

} // namespace mvp

#endif
