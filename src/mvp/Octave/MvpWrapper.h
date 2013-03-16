/// \file MvpWrapper.h
///
/// TODO: Write doc
///

#ifndef BOOST_PP_IS_ITERATING

#ifndef __MVP_OCTAVE_MVPWRAPPER_H__
#define __MVP_OCTAVE_MVPWRAPPER_H__

#include <mvp/Octave/Conversions.h>

#include <boost/preprocessor/repetition/enum_params_with_a_default.hpp>

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
struct MvpWrapperInstallerRegistrar {
  static MvpWrapperInstallerRegistrar<ImplT> reg;
  static octave_value_list construct_fcn(octave_value_list const& args, int nargout) {
    return mvp_wrapper<ImplT>(NULL, "", args);
  }
  MvpWrapperInstallerRegistrar(std::string name, std::string desc) {
    MvpWrapperInstaller::add_fcn_desc(construct_fcn, name, desc);
  }
};

#define MVP_WRAPPER_MAX_ARITY 10

template <class ImplT, class R, BOOST_PP_ENUM_PARAMS_WITH_A_DEFAULT(MVP_WRAPPER_MAX_ARITY, class T, void)>
struct mvp_wrapper_helper;

#define BOOST_PP_ITERATION_LIMITS (0, MVP_WRAPPER_MAX_ARITY)
#define BOOST_PP_FILENAME_1 <mvp/Octave/MvpWrapper.h>
#include BOOST_PP_ITERATE()

#define BEGIN_MVP_WRAPPER(NAME, IMPLT) \
template <> MvpWrapperInstallerRegistrar<IMPLT> MvpWrapperInstallerRegistrar<IMPLT>::reg(#NAME, std::string()); \
template <> \
octave_value mvp_wrapper<IMPLT>(IMPLT *impl, std::string const& func, octave_value_list const& args) { \
  using namespace mvp::octave; \
  typedef IMPLT ImplT; \
  try {

#define MVP_WRAP_args_helper(r, x, n, t) BOOST_PP_COMMA_IF(n) octave_as<t>(args(n))

#define MVP_WRAP_CONSTRUCTOR(ARGS) \
  if (!impl && args.length() == BOOST_PP_SEQ_SIZE(ARGS)) { \
    return octave_wrap(ImplT(BOOST_PP_SEQ_FOR_EACH_I(MVP_WRAP_args_helper, ~, ARGS))); \
  }

#define MVP_WRAP(FUNC, ARGS) \
  if (func == BOOST_PP_STRINGIZE(FUNC) && \
      args.length() == BOOST_PP_SUB(BOOST_PP_SEQ_SIZE(ARGS), 1)) {\
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    return mvp_wrapper_helper<ImplT, BOOST_PP_SEQ_ENUM(ARGS)>::wrap(impl, &ImplT::FUNC, args); \
  }

#define MVP_WRAP_C(FUNC, ARGS) MVP_WRAP(FUNC, ARGS)

#define END_MVP_WRAPPER() \
  } catch (vw::Exception &e) { \
    error("calling function %s: %s", func.c_str(), e.what()); \
    return octave_value(); \
  } \
  if (impl) { \
    error("function %s not defined, or wrong number of args", func.c_str()); \
  } else { \
    error("wrong number of args in constructor"); \
  } \
  return octave_value(); \
}

#endif

#else // BOOST_PP_IS_ITERATING

#define N BOOST_PP_ITERATION()
#define MVP_WRAP_args_helper2(r, n, data) mvp::octave::octave_as<typename boost::remove_reference<BOOST_PP_CAT(T, n)>::type>(args(n))

template <class ImplT, class R BOOST_PP_ENUM_TRAILING_PARAMS(N, class T)>
struct mvp_wrapper_helper
#if N != MVP_WRAPPER_MAX_ARITY
<ImplT, R BOOST_PP_ENUM_TRAILING_PARAMS(N, T)> 
#endif 
{
  template <class ImplBaseT>
  static octave_value wrap(ImplT *impl, 
                           R (ImplBaseT::*fcn)(BOOST_PP_ENUM_PARAMS(N, T)), 
                           octave_value_list const& args) {
    VW_ASSERT(args.length() == 0, vw::LogicErr() << "Unexpected number of args");
    return mvp::octave::octave_wrap((impl->*fcn)(BOOST_PP_ENUM(N, MVP_WRAP_args_helper2, ~)));
  }
  template <class ImplBaseT>
  static octave_value wrap(ImplT *impl, 
                           R (ImplBaseT::*fcn)(BOOST_PP_ENUM_PARAMS(N, T)) const,
                           octave_value_list const& args) {
    VW_ASSERT(args.length() == 0, vw::LogicErr() << "Unexpected number of args");
    return mvp::octave::octave_wrap((impl->*fcn)(BOOST_PP_ENUM(N, MVP_WRAP_args_helper2, ~)));
  }
};

template <class ImplT BOOST_PP_ENUM_TRAILING_PARAMS(N, class T)>
struct mvp_wrapper_helper<ImplT, void BOOST_PP_ENUM_TRAILING_PARAMS(N, T)> {
  template <class ImplBaseT>
  static octave_value wrap(ImplT *impl,
                           void (ImplBaseT::*fcn)(BOOST_PP_ENUM_PARAMS(N, T)),
                           octave_value_list const& args) {
    VW_ASSERT(args.length() == 0, vw::LogicErr() << "Unexpected number of args");
    (impl->*fcn)(BOOST_PP_ENUM(N, MVP_WRAP_args_helper2, ~));
    return octave_value();
  }
  template <class ImplBaseT>
  static octave_value wrap(ImplT *impl, void (ImplT::*fcn)(BOOST_PP_ENUM_PARAMS(N, T)) const, octave_value_list const& args) {
    VW_ASSERT(args.length() == 0, vw::LogicErr() << "Unexpected number of args");
    (impl->*fcn)(BOOST_PP_ENUM(N, MVP_WRAP_args_helper2, ~));
    return octave_value();
  }
};

#undef N
#undef MVP_WRAP_args_helper2

#endif // BOOST_PP_IS_ITERATING
