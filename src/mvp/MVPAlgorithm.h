/// \file MVPAlgorithm.h
///
/// MVPAlgorithm class
///
/// TODO: Write something here

#ifndef __MVP_MVPALGORITHM_H__
#define __MVP_MVPALGORITHM_H__

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <vw/Octave/Conversions.h>
#endif

namespace mvp {

struct MVPAlgorithmVar {
  vw::float32 height;

  vw::Vector3f orientation;
  vw::Vector3f windows;

  MVPAlgorithmVar(vw::float32 h, vw::Vector3f const& o, vw::Vector3f const& w) :
    height(h), orientation(o), windows(w) {}
  
  // to_octave()
};

struct MVPAlgorithmResult {
  MVPAlgorithmVar result;
  vw::float32 variance;
  bool converged;
  int num_iterations_to_converge;

  MVPAlgorithmResult(MVPAlgorithmVar r, vw::float32 v = 0f, bool c = true, int n = 0) :
    result(r), variance(v), converged(c), num_iterations_to_converge(n) {}

  // constructor from octave vars
};

class MVPAlgorithmImplBase {
  protected:
    MVPAlgorithmSettings m_settings;

  public:
    MVPAlgorithmImplBase(MVPAlgorithmSettings const& settings) : m_settings(settings) {}
  
    virtual const MVPAlgorithmResult do_algorithm(MVPAlgorithmVar const& seed, 
                                                  vw::cartography::GeoReference const& georef) const = 0;
};

class MVPAlgorithmImpl : public MVPAlgorithmImplBase {
  OrbitalImageCropCollection m_images;

  public:
    MVPAlgorithmImpl(MVPAlgorithmSettings const& settings, OrbitalImageCropCollection const& images) :
      MVPAlgorithmImplBase(settings), m_images(images) {}

    virtual const MVPAlgorithmResult do_algorithm(MVPAlgorithmVar const& seed, 
                                                  vw::cartography::GeoReference const& georef) const
    {
      return MVPAlgorithmResult(seed);
    }
};

#if MVP_ENABLE_OCTAVE_SUPPORT
class MVPAlgorithmImplOctave : public MVPAlgorithmImplBase {
  // Octave OrbitalImageCropCollection

  public:
    MVPAlgorithmImpl(MVPAlgorithmSettings const& settings, OrbitalImageCropCollection const& images) :
      MVPAlgorithmImplBase(settings) {}

    virtual const MVPAlgorithmResult do_algorithm(MVPAlgorithmVar const& seed, 
                                                  vw::cartography::GeoReference const& georef) const
    {
      return MVPAlgorithmResult(seed);
    }
};
#endif

class MVPAlgorithm {
  boost::shared_ptr<MVPAlgorithmImplBase> m_impl;

  public:
    MVPAlgorithm(MVPAlgorithmSettings const& settings, OrbitalImageCropCollection const& images) {
      if (settings.use_octave()) {
        #if MVP_ENABLE_OCTAVE_SUPPORT
          m_impl.reset(new MVPAlgorithmImplOctave(settings, images));
        #else
          vw::vw_throw(vw::NoImplErr() << "Cannot use octave algorithm, as the MVP was not compled with it!");
        #endif
      } else {
        m_impl.reset(new MVPAlgorithmImpl(settings, images)); 
      }
    }

    const MVPAlgorithmResult operator()(MVPAlgorithmVar const& seed, 
                                        vw::cartography::GeoReference const& georef) const {
      return m_impl->do_algorithm(seed, georef);
    }
};


} // namespace mvp

#endif
