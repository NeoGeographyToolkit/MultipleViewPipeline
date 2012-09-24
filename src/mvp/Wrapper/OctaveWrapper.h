/// \file OctaveWrapper.h
///
/// Wrap octave classes so they can be
/// accessed in C++
///

#ifndef __MVP_WRAPPER_OCTAVEWRAPPER_H__
#define __MVP_WRAPPER_OCTAVEWRAPPER_H__

#include <mvp/Octave/oct-cast.h>

namespace mvp {
namespace wrapper {

class OctaveWrapperImpl {
  octave_value m_impl;

  public:
    OctaveWrapperImpl(octave_value const& impl) : m_impl(impl) {}

    OctaveWrapperImpl(std::string const& impl_name, octave_value_list const& args = octave_value_list());

    octave_value wrap_function(std::string const& func, octave_value_list const& args = octave_value_list());

    template <class T1>
    octave_value wrap_function1(std::string const& func, T1 arg1) {
      octave_value_list args;
      args.append(mvp::octave::octave_cast<octave_value>(arg1));
      return wrap_function(func, args);
    }

    template <class T1, class T2>
    octave_value wrap_function2(std::string const& func, T1 arg1, T2 arg2) {
      octave_value_list args;
      args.append(mvp::octave::octave_cast<octave_value>(arg1));
      args.append(mvp::octave::octave_cast<octave_value>(arg2));
      return wrap_function(func, args);
    }
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

#define OCT_WRAPPER_FUNCTION(RT, NAME) \
  RT NAME() { \
    return mvp::octave::octave_cast<RT>(m_wrap.wrap_function(#NAME)); \
  }

#define OCT_WRAPPER_FUNCTION1(RT, NAME, T1) \
  RT NAME(T1 arg1) { \
    return mvp::octave::octave_cast<RT>(m_wrap.wrap_function1(#NAME, arg1)); \
  }

#define OCT_WRAPPER_FUNCTION2(RT, NAME, T1, T2) \
  RT NAME(T1 arg1, T2 arg2) { \
    return mvp::octave::octave_cast<RT>(m_wrap.wrap_function1(#NAME, arg1, arg2)); \
  }

#define OCT_WRAPPER_VOID(NAME) \
  void NAME() { \
    m_wrap.wrap_function2(#NAME); \
  }

#define OCT_WRAPPER_VOID1(NAME, T1) \
  void NAME(T1 arg1) { \
    m_wrap.wrap_function2(#NAME, arg1); \
  }

#define OCT_WRAPPER_VOID2(NAME, T1, T2) \
  void NAME(T1 arg1, T2 arg2) { \
    m_wrap.wrap_function2(#NAME, arg1, arg2); \
  }

#define OCT_WRAPPER_END() ;

#endif
