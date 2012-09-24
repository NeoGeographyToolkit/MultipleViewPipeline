/// \file MvpWrapper.h
///
/// MVP_WRAPPER_* defines
///

#ifndef __MVP_WRAPPER_MVPWRAPPER_H__
#define __MVP_WRAPPER_MVPWRAPPER_H__

#include <mvp/Octave/oct-mvpobj.h>
#include <mvp/Octave/OctaveCast.h>

/// MVP_WRAPPER_CONSTRUCTOR

#define MVP_WRAPPER_BEGINC(NAME, TYPE) \
octave_value_list NAME(octave_value_list const& args, int nargout) { \
  typedef TYPE type; \
  if (args.length() == 0) { \
    return octave_value(new octave_mvpobj_ref(new octave_mvpobj_impl<type>())); \
  } \
  type *obj = NULL; \
  std::string name = args(0).string_value(); \
  try

#define MVP_WRAPPER_CONSTRUCTOR(CNAME) \
  if (name == CNAME)

#define MVP_WRAPPER_CONSTRUCTOR_RETURN(EXPR) \
  obj = new EXPR

#define MVP_WRAPPER_ENDC() \
  catch (vw::Exception &e) { \
    error("invalid cast: %s", e.what()); \
    return octave_value(); \
  } \
  if (obj) { \
    return octave_value(new octave_mvpobj_ref(new octave_mvpobj_wrap<type>(obj, true))); \
  } else { \
    error("`%s' is not a valid classname", name.c_str()); \
    return octave_value(); \
  } \
}

/// MVP_WRAPPER_FUNCTION

#define MVP_WRAPPER_BEGIN(TYPE) \
octave_value mvpobj_wrap_function(TYPE *obj, std::string const& func, octave_value_list const& args) { \
  try

#define MVP_WRAPPER_void \

#define MVP_WRAPPER_func \
ret = mvp::octave::octave_cast<octave_value>

#define MVP_WRAPPER_0arg(NAME, VOID_OR_FUNC) \
    if (func == #NAME) { \
      if (args.length() == 0) { \
        octave_value ret; \
        VOID_OR_FUNC(obj->NAME()); \
        return ret;\
      } \
    }

#define MVP_WRAPPER_1arg(NAME, T1, VOID_OR_FUNC) \
    if (func == #NAME) { \
      if (args.length() == 1) { \
        octave_value ret; \
        T1 arg1 = mvp::octave::octave_cast<T1>(args(0)); \
        VOID_OR_FUNC(obj->NAME(arg1)); \
        return ret;\
      } \
    }

#define MVP_WRAPPER_2arg(NAME, T1, T2, VOID_OR_FUNC) \
    if (func == #NAME) { \
      if (args.length() == 2) { \
        octave_value ret; \
        T1 arg1 = mvp::octave::octave_cast<T1>(args(0)); \
        T2 arg2 = mvp::octave::octave_cast<T2>(args(1)); \
        VOID_OR_FUNC(obj->NAME(arg1, arg2)); \
        return ret;\
      } \
    }

#define MVP_WRAPPER_END() \
    catch (vw::Exception &e) { \
    error("invalid cast: %s", e.what()); \
  } \
  error("unable to call function %s", func.c_str()); \
  return octave_value(); \
}

#define MVP_WRAPPER_FUNCTION(RT, NAME) \
  MVP_WRAPPER_0arg(NAME, MVP_WRAPPER_func)

#define MVP_WRAPPER_FUNCTION1(RT, NAME, T1) \
  MVP_WRAPPER_1arg(NAME, T1, MVP_WRAPPER_func)

#define MVP_WRAPPER_FUNCTION2(RT, NAME, T1, T2) \
  MVP_WRAPPER_2arg(NAME, T1, T2, MVP_WRAPPER_func)

#define MVP_WRAPPER_VOID(NAME) \
  MVP_WRAPPER_0arg(NAME, MVP_WRAPPER_void)

#define MVP_WRAPPER_VOID1(NAME, T1) \
  MVP_WRAPPER_1arg(NAME, T1, MVP_WRAPPER_void)

#define MVP_WRAPPER_VOID2(NAME, T1, T2) \
  MVP_WRAPPER_2arg(NAME, T1, T2, MVP_WRAPPER_void)

#endif
