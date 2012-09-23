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

#define OCT_WRAPPER_BEGIN(NAME, TYPE) \
typedef OctaveWrapper<TYPE> NAME; \
template <> \
class OctaveWrapper<TYPE> : public TYPE

#define OCT_WRAPPER_INIT() \
  OctaveWrapperImpl m_wrap; \
  public: \
    OctaveWrapper(octave_value const& impl) : m_wrap(impl) {} \
    OctaveWrapper(std::string const& impl_name) : m_wrap(impl_name) {} \
    OctaveWrapper(char const* impl_name) : m_wrap(std::string(impl_name)) {} 


#define OCT_WRAPPER_FUNCTION_INIT() \
      octave_value_list args;

#define OCT_WRAPPER_FUNCTION_ARG(ARG) \
      args.append(mvp::octave::octave_cast<octave_value>(ARG));

#define OCT_WRAPPER_FUNCTION_VOID(NAME) \
      m_wrap.wrap_function(NAME, args);

#define OCT_WRAPPER_FUNCTION(TYPE, NAME) \
       mvp::octave::octave_cast<TYPE>(m_wrap.wrap_function(NAME, args));

#define OCT_WRAPPER_END() ;

#endif
