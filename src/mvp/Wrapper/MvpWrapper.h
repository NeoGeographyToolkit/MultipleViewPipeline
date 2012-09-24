/// \file MvpWrapper.h
///
/// MVP_WRAPPER_* defines
///

#ifndef __MVP_WRAPPER_MVPWRAPPER_H__
#define __MVP_WRAPPER_MVPWRAPPER_H__

#include <mvp/Octave/oct-mvpobj.h>
#include <mvp/Octave/oct-cast.h>

#define MVP_WRAPPER_BEGIN(NAME, TYPE) \
template <> \
octave_value mvp_wrapper<TYPE>(TYPE *obj, std::string const& func, octave_value_list const& args); \
\
octave_value_list NAME(octave_value_list const& args, int nargout) { \
  if (args.length() == 0) { \
    return octave_value(new octave_mvpobj_ref(new octave_mvpobj_impl<TYPE>())); \
  } \
  std::string name = args(0).string_value(); \
  return mvp_wrapper<TYPE>(0, name, args.slice(1, args.length() - 1)); \
} \
\
template <> \
octave_value mvp_wrapper<TYPE>(TYPE *obj, std::string const& func, octave_value_list const& args) { \
  typedef TYPE type; \
  try

#define MVP_WRAPPER_CONSTRUCT(CLASS, NAME) \
    if (!obj) { \
      if (func == NAME) { \
        if (args.length() == 0) { \
          return octave_value(new octave_mvpobj_ref(new octave_mvpobj_wrap<type>(new CLASS(), true))); \
        } \
      } \
    }

#define MVP_WRAPPER_CONSTRUCT1(CLASS, NAME, T1) \
    if (!obj) { \
      if (func == NAME) { \
        if (args.length() == 1) { \
          T1 arg1 = mvp::octave::octave_cast<T1>(args(0)); \
          return octave_value(new octave_mvpobj_ref(new octave_mvpobj_wrap<type>(new CLASS(arg1), true))); \
        } \
      } \
    }

#define MVP_WRAPPER_CONSTRUCT2(CLASS, NAME, T1, T2) \
    if (!obj) { \
      if (func == NAME) { \
        if (args.length() == 2) { \
          T1 arg1 = mvp::octave::octave_cast<T1>(args(0)); \
          T2 arg2 = mvp::octave::octave_cast<T2>(args(1)); \
          return octave_value(new octave_mvpobj_ref(new octave_mvpobj_wrap<type>(new CLASS(arg1, arg2), true))); \
        } \
      } \
    }

#define MVP_WRAPPER_void

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

#define MVP_WRAPPER_END \
    catch (vw::Exception &e) { \
    error("invalid cast: %s", e.what()); \
  } \
  error("invalid function name `%s' or wrong number of args", func.c_str()); \
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
