/// \file OctaveWrapper.h
///
/// Wrap octave classes so they can be
/// accessed in C++
///

#ifndef __MVP_OCTAVE_OCTAVEWRAPPER_H__
#define __MVP_OCTAVE_OCTAVEWRAPPER_H__

#include <mvp/Octave/Conversions.h>

namespace mvp {
namespace octave {

class OctaveWrapperImpl {
  octave_value m_impl;

  static octave_value create_impl(std::string const& impl_name, octave_value_list const& args);

  octave_value wrap_function_impl(std::string const& func, octave_value_list const& args);

  public:
    OctaveWrapperImpl(octave_value const& impl) : m_impl(impl) {}

    OctaveWrapperImpl(std::string const& impl_name) {
      m_impl = create_impl(impl_name, octave_value_list());
    }

    template <class T1>
    OctaveWrapperImpl(std::string const& impl_name, T1 arg1) {
      octave_value_list args;
      args.append(octave_wrap(arg1));
      m_impl = create_impl(impl_name, args);
    }

    template <class T1, class T2>
    OctaveWrapperImpl(std::string const& impl_name, T1 arg1, T2 arg2) {
      octave_value_list args;
      args.append(octave_wrap(arg1));
      args.append(octave_wrap(arg2));
      m_impl = create_impl(impl_name, args);
    }

    octave_value wrap_function(std::string const& func) {
      return wrap_function_impl(func, octave_value_list());
    }

    template <class T1>
    octave_value wrap_function(std::string const& func, T1 arg1) {
      octave_value_list args;
      args.append(octave_wrap(arg1));
      return wrap_function_impl(func, args);
    }

    template <class T1, class T2>
    octave_value wrap_function(std::string const& func, T1 arg1, T2 arg2) {
      octave_value_list args;
      args.append(octave_wrap(arg1));
      args.append(octave_wrap(arg2));
      return wrap_function_impl(func, args);
    }
};

template <class BaseT>  
class OctaveWrapper : public BaseT {
  // Nobody should construct this
  OctaveWrapper() {}
  public:
    virtual ~OctaveWrapper() {}
};

}} // namespace octave, mvp

#endif
