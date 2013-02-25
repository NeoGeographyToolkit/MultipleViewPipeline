/// \file MvpWrapper.h
///
/// TODO: Write doc
///

#ifndef __MVP_OCTAVE_MVPWRAPPER_H__
#define __MVP_OCTAVE_MVPWRAPPER_H__

#include <mvp/Octave/Conversions.h>
#include <boost/preprocessor/facilities/overload.hpp>

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

#define MVP_WRAP_args(r, x, n, t) BOOST_PP_COMMA_IF(n) octave_as<t>(args(n))

#define MVP_WRAP_CONSTRUCTOR_0() \
  if (!impl && args.length() == 0) { \
    return octave_wrap(ImplT()); \
  }

#define MVP_WRAP_CONSTRUCTOR_1(SIG) \
  if (!impl && args.length() == BOOST_PP_SEQ_SIZE(SIG)) { \
    return octave_wrap(ImplT(BOOST_PP_SEQ_FOR_EACH_I(MVP_WRAP_args, ~, SIG))); \
  }

#define MVP_WRAP_CONSTRUCTOR(...) BOOST_PP_OVERLOAD(MVP_WRAP_CONSTRUCTOR_, __VA_ARGS__)(__VA_ARGS__)

#define MVP_WRAP_FUNCTION_0(FUNC) \
  if (func == #FUNC && args.length() == 0) { \
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    return octave_wrap(impl->FUNC()); \
  }

#define MVP_WRAP_FUNCTION_1(FUNC, SIG) \
  if (func == #FUNC && args.length() == BOOST_PP_SEQ_SIZE(SIG)) { \
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    return octave_wrap(impl->FUNC(BOOST_PP_SEQ_FOR_EACH_I(MVP_WRAP_args, ~, SIG))); \
  }

#define MVP_WRAP_FUNCTION(FUNC, ...) BOOST_PP_OVERLOAD(MVP_WRAP_FUNCTION_, __VA_ARGS__)(FUNC, __VA_ARGS__)

#define MVP_WRAP_VOID_0(FUNC) \
  if (func == #FUNC && args.length() == 0) { \
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    impl->FUNC(); \
    return octave_value(); \
  }

#define MVP_WRAP_VOID_1(FUNC, SIG) \
  if (func == #FUNC && args.length() == BOOST_PP_SEQ_SIZE(SIG)) { \
    VW_ASSERT(impl, vw::LogicErr() << "impl not defined!"); \
    impl->FUNC(BOOST_PP_SEQ_FOR_EACH_I(MVP_WRAP_args, ~, SIG)); \
    return octave_value(); \
  }

#define MVP_WRAP_VOID(FUNC, ...) BOOST_PP_OVERLOAD(MVP_WRAP_VOID_, __VA_ARGS__)(FUNC, __VA_ARGS__)

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
