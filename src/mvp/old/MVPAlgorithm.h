/// \file MVPAlgorithm.h
///
/// MVP Algorithm
///
/// TODO: Write something here
///

#include <mvp/MVPTypes.h>
#include <mvp/OrbitalImageCrop.h>
#include <mvp/MVPAlgorithmOptions.pb.h>

#ifndef __MVP_MVPALGORITHM_H__
#define __MVP_MVPALGORITHM_H__

namespace mvp {

struct MVPAlgorithm {
  virtual MVPPixelResult operator()(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const = 0;
};

class MVPAlgoImpl : public MVPAlgorithm {
  OrbitalImageCropCollection m_crops;
  public:
    MVPAlgoImpl(OrbitalImageCropCollection const& crops) : m_crops(crops) {}
    virtual MVPPixelResult operator()(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const;
};

class MVPAlgoFootprint : public MVPAlgorithm {
  OrbitalImageCropCollection m_crops;
  public:
    MVPAlgoFootprint(OrbitalImageCropCollection const& crops) : m_crops(crops) {}
    virtual MVPPixelResult operator()(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const;
};

#if MVP_ENABLE_OCTAVE_SUPPORT
class MVPAlgoOctave : public MVPAlgorithm {
  ::octave_map m_crops;
  std::string m_octave_fcn;
  public:
    MVPAlgoOctave(OrbitalImageCropCollection const& crops, std::string octave_fcn) : m_crops(crops.to_octave()), m_octave_fcn(octave_fcn) {}
    virtual MVPPixelResult operator()(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const;
};
#endif

} // namespace mvp

#endif
