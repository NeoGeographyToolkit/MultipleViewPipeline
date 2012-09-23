/// \file OctaveWrapper.h
///
/// Wrap octave classes so they can be
/// accessed in C++
///

#ifndef __MVP_WRAPPER_OCTAVEWRAPPER_H__
#define __MVP_WRAPPER_OCTAVEWRAPPER_H__

#include <mvp/Octave/OctaveCast.h>

namespace mvp {
namespace wrapper {

class OctaveWrapperImpl {
  octave_value m_impl;

  public:
    OctaveWrapperImpl(octave_value const& impl) : m_impl(impl) {}

    OctaveWrapperImpl(std::string const& impl_name, octave_value_list const& args = octave_value_list());

    octave_value wrap_function(std::string const& func, octave_value_list const& args = octave_value_list());
};

template <class BaseT>  
class OctaveWrapper : BaseT {
  private:
    // Nobody should construct this
    OctaveWrapper() {}
};

}} // namespace wrapper, mvp

#endif
