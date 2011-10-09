/// \file MVPAlgorithm.h
///
/// MVPAlgorithm class
///
/// TODO: Write something here

#ifndef __MVPALGORITHM_H__
#define __MVPALGORITHM_H__

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
  OrbitalImageCollection m_images;

  public:
    MVPAlgorithmImpl(MVPAlgorithmSettings const& settings, OrbitalImageCollection const& images) :
      MVPAlgorithmImplBase(settings), m_images(images) {}

    virtual const MVPAlgorithmResult do_algorithm(MVPAlgorithmVar const& seed, 
                                                  vw::cartography::GeoReference const& georef) const
    {
      return MVPAlgorithmResult(seed);
    }
};

class MVPAlgorithmImplOctave : public MVPAlgorithmImplBase {
  // Octave OrbitalImageCollection

  public:
    MVPAlgorithmImpl(MVPAlgorithmSettings const& settings, OrbitalImageCollection const& images) :
      MVPAlgorithmImplBase(settings) {}

    virtual const MVPAlgorithmResult do_algorithm(MVPAlgorithmVar const& seed, 
                                                  vw::cartography::GeoReference const& georef) const
    {
      return MVPAlgorithmResult(seed);
    }
};

class MVPAlgorithm {
  boost::shared_ptr<MVPAlgorithmImplBase> m_impl;

  public:
    MVPAlgorithm(MVPAlgorithmSettings const& settings, OrbitalImageCollection const& images) {
      m_impl.reset(new MVPAlgorithmImpl(settings, images)); 
    }

    const MVPAlgorithmResult operator()(MVPAlgorithmVar const& seed, 
                                        vw::cartography::GeoReference const& georef) const {
      return m_impl->do_algorithm(seed, georef);
    }
};


} // namespace mvp

#endif
