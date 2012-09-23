/// \file OctaveStepper.h
///
/// TODO: Write doc
///

#ifndef __MVP_WRAPPER_OCTAVESTEPPER_H__
#define __MVP_WRAPPER_OCTAVESTEPPER_H__

#include <mvp/Wrapper/OctaveWrapper.h>

#include <mvp/Algorithm/Stepper.h>

namespace mvp {
namespace wrapper {

template <>
class OctaveWrapper<algorithm::Stepper> : public algorithm::Stepper {
  OctaveWrapperImpl m_wrap;

  public:
    OctaveWrapper(octave_value const& impl) : m_wrap(impl) {}

    OctaveWrapper(std::string const& impl_name) : m_wrap(impl_name) {}
    
    OctaveWrapper(char const* impl_name) : m_wrap(std::string(impl_name)) {}

    void zing(int a, int b) {
      using mvp::octave::octave_cast;
      octave_value_list args;
      args.append(octave_cast<octave_value>(a));
      args.append(octave_cast<octave_value>(b));
      m_wrap.wrap_function("zing", args);
    }

    int zap(int a) {
      using mvp::octave::octave_cast;
      octave_value_list args;
      args.append(octave_cast<octave_value>(a));
      return octave_cast<int>(m_wrap.wrap_function("zap", args));
    }
};

typedef OctaveWrapper<algorithm::Stepper> OctaveStepper;

}} // namespace wrapper,mvp

#endif
