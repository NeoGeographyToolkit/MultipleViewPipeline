/// \file MvpWrapper.h
///
/// TODO: Write doc
///

#ifndef BOOST_PP_IS_ITERATING

#ifndef __MVP_OCTAVE_MVPWRAPPER_H__
#define __MVP_OCTAVE_MVPWRAPPER_H__

#include <mvp/Octave/Conversions.h>
#include <mvp/Octave/oct-mvpclass.h>

#define BEGIN_MVP_WRAPPER(IMPLT) \
template <> \
octave_value mvp_wrapper<IMPLT>(IMPLT *impl, std::string const& func, octave_value_list const& args) { \
  using namespace mvp::octave; \
  typedef IMPLT ImplT; \
  try {

#define MVP_WRAP(FUNC) \
  if (func == #FUNC) { \
    return mvp_wrap_function(impl, &ImplT::FUNC, args); \
  }

#define END_MVP_WRAPPER() \
  } catch (vw::Exception &e) { \
    error("error calling function: %s", e.what()); \
    return octave_value(); \
  } \
  error("function %s not defined", func.c_str()); \
  return octave_value(); \
}

// put defs here

#define MVP_WRAPPER_MAX_ARITY 10
#define BOOST_PP_ITERATION_LIMITS (0, MVP_WRAPPER_MAX_ARITY)
#define BOOST_PP_FILENAME_1 <mvp/Octave/MvpWrapper.h>
#include BOOST_PP_ITERATE()

#endif

#else // BOOST_PP_IS_ITERATING

#define N BOOST_PP_ITERATION()
#define PARAM_CAST(z, n, T) octave_as<T##n>(args(n))

template <class ImplT, class R BOOST_PP_ENUM_TRAILING_PARAMS(N, class T)>
octave_value mvp_wrap_function(ImplT *impl, R (ImplT::*f)(BOOST_PP_ENUM_PARAMS(N, T)), octave_value_list const& args) {
  using namespace mvp::octave;

  if (args.length() != N) {
    error("invalid number of args");
    return octave_value();
  }

  /*
  (impl->*f)(BOOST_PP_ENUM(N, PARAM_CAST, T));
  return octave_value();
  */

  return (impl->*f)(BOOST_PP_ENUM(N, PARAM_CAST, T));
}

#undef PARAM_CAST
#undef N

#endif
