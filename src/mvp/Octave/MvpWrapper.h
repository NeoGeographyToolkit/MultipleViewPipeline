/// \file MvpWrapper.h
///
/// TODO: Write doc
///

#ifndef BOOST_PP_IS_ITERATING

#ifndef __MVP_OCTAVE_MVPWRAPPER_H__
#define __MVP_OCTAVE_MVPWRAPPER_H__

#include <mvp/Octave/Conversions.h>
#include <boost/preprocessor/control/expr_if.hpp>
#include <boost/preprocessor/iteration/iterate.hpp>
#include <boost/preprocessor/repetition/enum.hpp>
#include <boost/preprocessor/repetition/enum_params.hpp>
#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>
#include <boost/preprocessor/repetition/enum_trailing_params.hpp>

#include <boost/type_traits/remove_cv.hpp>

class MvpWrapperInstaller {
  struct OctaveFcnDesc {
    octave_builtin::fcn f;
    std::string name;
    std::string desc; 
    OctaveFcnDesc(octave_builtin::fcn _f, std::string _name, std::string _desc)
      : f(_f), name(_name), desc(_desc) {}
  };

  static std::vector<OctaveFcnDesc>& installer_descs();

  public:
    static void install_wrappers();

    static void add_fcn_desc(octave_builtin::fcn f, std::string name, std::string desc);
};

template <class ImplT>
class MvpWrapperInstallerRegistrar {
  static MvpWrapperInstallerRegistrar<ImplT> reg;
  static octave_value_list construct_fcn(octave_value_list const& args, int nargout) {
    return mvp_wrapper<ImplT>(NULL, "", args);
  }
  public:
    MvpWrapperInstallerRegistrar(std::string name, std::string desc) {
      MvpWrapperInstaller::add_fcn_desc(construct_fcn, name, desc);
    }
};

#define BEGIN_MVP_WRAPPER(NAME, IMPLT) \
template <> MvpWrapperInstallerRegistrar<IMPLT> MvpWrapperInstallerRegistrar<IMPLT>::reg(#NAME, std::string()); \
template <> \
octave_value mvp_wrapper<IMPLT>(IMPLT *impl, std::string const& func, octave_value_list const& args) { \
  using namespace mvp::octave; \
  typedef IMPLT ImplT; \
  try {

#define MVP_WRAP_construct_args(r, x, n, t) BOOST_PP_COMMA_IF(n) octave_as<t>(args(n))

#define MVP_WRAP_CONSTRUCTOR(SIG) \
  if (!impl && args.length() == BOOST_PP_SEQ_SIZE(SIG)) { \
    return octave_wrap(ImplT(BOOST_PP_SEQ_FOR_EACH_I(MVP_WRAP_construct_args, ~, SIG))); \
  }

#define MVP_WRAP(FUNC) \
  if (func == #FUNC) { \
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    return mvp_wrap_function(impl, &ImplT::FUNC, args); \
  }

#define END_MVP_WRAPPER() \
  } catch (vw::Exception &e) { \
    error("error calling function: %s", e.what()); \
    return octave_value(); \
  } \
  if (impl) { \
    error("function %s not defined", func.c_str()); \
  } else { \
    error("wrong number of args in constructor"); \
  } \
  return octave_value(); \
}

// put defs here

#define MVP_WRAPPER_MAX_ARITY 10

template <class ImplT, class R, BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(MVP_WRAPPER_MAX_ARITY, class T, void)>
struct MvpWrapperHelper;

#define BOOST_PP_ITERATION_LIMITS (0, MVP_WRAPPER_MAX_ARITY)
#define BOOST_PP_FILENAME_1 <mvp/Octave/MvpWrapper.h>
#include BOOST_PP_ITERATE()

#endif

#else // BOOST_PP_IS_ITERATING

#define N BOOST_PP_ITERATION()

#define MVP_WRAPPER_param_cast(z, n, data) octave_as<typename boost::remove_cv<typename boost::remove_reference<T##n>::type>::type>(args(n))

template <class ImplT, class R BOOST_PP_ENUM_TRAILING_PARAMS(N,class T)>
octave_value mvp_wrap_function(ImplT *impl, 
                               R (ImplT::*f)(BOOST_PP_ENUM_PARAMS(N, T)),
                               octave_value_list const& args) {
  if (args.length() != N) {
    error("invalid number of args");
    return octave_value();
  } 
  return MvpWrapperHelper<ImplT, R BOOST_PP_ENUM_TRAILING_PARAMS(N,T)>::wrap(impl, f, args);
}

template <class ImplT, class R BOOST_PP_ENUM_TRAILING_PARAMS(N,class T)>
octave_value mvp_wrap_function(ImplT *impl, 
                               R (ImplT::*f)(BOOST_PP_ENUM_PARAMS(N, T)) const,
                               octave_value_list const& args) {
  if (args.length() != N) {
    error("invalid number of args");
    return octave_value();
  } 
  return MvpWrapperHelper<ImplT, R BOOST_PP_ENUM_TRAILING_PARAMS(N,T)>::wrap(impl, f, args);
}

template <class ImplT, class R BOOST_PP_ENUM_TRAILING_PARAMS(N,class T)>
struct MvpWrapperHelper
#if N != MVP_WRAPPER_MAX_ARITY
<ImplT,R BOOST_PP_ENUM_TRAILING_PARAMS(N,T)>
#endif
{
  static octave_value wrap(ImplT *impl, 
                           R (ImplT::*f)(BOOST_PP_ENUM_PARAMS(N,T)),
                           octave_value_list const& args) {
    using namespace mvp::octave;
    return octave_wrap((impl->*f)(BOOST_PP_ENUM(N, MVP_WRAPPER_param_cast, ~)));
  }
  static octave_value wrap(ImplT *impl, 
                           R (ImplT::*f)(BOOST_PP_ENUM_PARAMS(N,T)) const,
                           octave_value_list const& args) {
    using namespace mvp::octave;
    return octave_wrap((impl->*f)(BOOST_PP_ENUM(N, MVP_WRAPPER_param_cast, ~)));
  }
};

template <class ImplT BOOST_PP_ENUM_TRAILING_PARAMS(N, class T)>
struct MvpWrapperHelper<ImplT, void BOOST_PP_ENUM_TRAILING_PARAMS(N, T)>
{
  static octave_value wrap(ImplT *impl, 
                           void (ImplT::*f)(BOOST_PP_ENUM_PARAMS(N, T)),
                           octave_value_list const& args) {
    using namespace mvp::octave;
    (impl->*f)(BOOST_PP_ENUM(N, MVP_WRAPPER_param_cast, ~));
    return octave_value();
  }
  static octave_value wrap(ImplT *impl, 
                           void (ImplT::*f)(BOOST_PP_ENUM_PARAMS(N, T)) const,
                           octave_value_list const& args) {
    using namespace mvp::octave;
    (impl->*f)(BOOST_PP_ENUM(N, MVP_WRAPPER_param_cast, ~));
    return octave_value();
  }
};

#undef MVP_WRAPPER_param_cast
#undef N

#endif
