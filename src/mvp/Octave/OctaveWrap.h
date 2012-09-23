/// \file OctaveWrap.h
///
/// OCT_WRAP_* defines
///

#ifndef __MVP_OCTAVE_OCTAVEWRAP_H__
#define __MVP_OCTAVE_OCTAVEWRAP_H__

#include <mvp/Octave/oct-mvpobj.h>
#include <mvp/Octave/OctaveCast.h>

/// OCT_WRAP_CONSTRUCTOR

#define OCT_WRAP_BEGINC(NAME, TYPE) \
octave_value_list NAME(octave_value_list const& args, int nargout) { \
  typedef TYPE type; \
  if (args.length() == 0) { \
    return octave_value(new octave_mvpobj_ref(new octave_mvpobj_impl<type>())); \
  } \
  type *obj = NULL; \
  std::string name = args(0).string_value(); \
  try

#define OCT_WRAP_CONSTRUCTOR(CNAME) \
  if (name == CNAME)

#define OCT_WRAP_CONSTRUCTOR_RETURN(EXPR) \
  obj = new EXPR

#define OCT_WRAP_ENDC() \
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

/// OCT_WRAP_FUNCTION

#define OCT_WRAP_BEGINF(TYPE) \
octave_value mvpobj_wrap_function(TYPE *obj, std::string const& func, octave_value_list const& args) { \
  try

#define OCT_WRAP_FUNCTION(FNAME) \
    if (func == FNAME) \

#define OCT_WRAP_FUNCTION_NARGS(NARGS) \
      if (args.length() != NARGS) { \
        error("%s requires %d args", func.c_str(), NARGS); \
        return octave_value(); \
      } \
      int currarg = 0;

#define OCT_WRAP_FUNCTION_ARG(ARGTYPE, ARGNAME) \
      ARGTYPE ARGNAME = mvp::octave::octave_cast<ARGTYPE>(args(currarg++));

#define OCT_WRAP_FUNCTION_RETURN(EXPR) \
      return mvp::octave::octave_cast<octave_value>(EXPR);

#define OCT_WRAP_FUNCTION_VOID(EXPR) \
      EXPR; \
      return octave_value();

#define OCT_WRAP_ENDF() \
    catch (vw::Exception &e) { \
    error("invalid cast: %s", e.what()); \
  } \
  error("unable to call function %s", func.c_str()); \
  return octave_value(); \
}

#endif
