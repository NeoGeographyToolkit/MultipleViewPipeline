/// \file MvpWrapper.h
///
/// TODO: Write doc
///

#ifndef __MVP_OCTAVE_MVPWRAPPER_H__
#define __MVP_OCTAVE_MVPWRAPPER_H__

#include <mvp/Octave/Conversions.h>

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

#define MVP_WRAP_FUNCTION_IMPL(FUNC, ARGS) \
  if (func == BOOST_PP_STRINGIZE(FUNC) && args.length() == BOOST_PP_SEQ_SIZE(ARGS)) { \
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    return octave_wrap(impl->FUNC(BOOST_PP_SEQ_FOR_EACH_I(MVP_WRAP_args_helper, ~, ARGS))); \
  }

#define MVP_WRAP_FUNCTION(SIG) \
  MVP_WRAP_FUNCTION_IMPL(BOOST_PP_SEQ_HEAD(SIG), BOOST_PP_SEQ_TAIL(SIG))

#define MVP_WRAP_VOID_IMPL(FUNC, ARGS) \
  if (func == BOOST_PP_STRINGIZE(FUNC) && args.length() == BOOST_PP_SEQ_SIZE(ARGS)) { \
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    impl->FUNC(BOOST_PP_SEQ_FOR_EACH_I(MVP_WRAP_args_helper, ~, ARGS)); \
    return octave_value(); \
  }

#define MVP_WRAP_VOID(SIG) \
  MVP_WRAP_VOID_IMPL(BOOST_PP_SEQ_HEAD(SIG), BOOST_PP_SEQ_TAIL(SIG))

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

#endif
