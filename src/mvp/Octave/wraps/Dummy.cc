#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Algorithm/Dummy.h>
#include <mvp/Octave/OctaveWrapper.h>

#include <boost/preprocessor/control/expr_if.hpp>

BEGIN_OCTAVE_WRAPPER(Dummy, mvp::algorithm::Dummy)
  OCTAVE_WRAP_CONSTRUCTOR((int)(int))
  OCTAVE_WRAP(void0, (void))
  OCTAVE_WRAP(void1, (void)(int))
  OCTAVE_WRAP(void2, (void)(int)(int))
  OCTAVE_WRAP(function0, (int))
  OCTAVE_WRAP(function1, (int)(int))
  OCTAVE_WRAP(function2, (int)(int)(int))
  OCTAVE_WRAP_C(x, (int))
  OCTAVE_WRAP_C(y, (int))
  OCTAVE_WRAP(do_vector, (vw::Vector2)(vw::Vector3 const&))
END_OCTAVE_WRAPPER()

BEGIN_MVP_WRAPPER(Dummy, mvp::algorithm::Dummy)
  MVP_WRAP_CONSTRUCTOR((std::string)(bool)(int)(int))
  MVP_WRAP(void0, (void))
  MVP_WRAP(void1, (void)(int))
  MVP_WRAP(void2, (void)(int)(int))
  MVP_WRAP(function0, (int))
  MVP_WRAP(function1, (int)(int))
  MVP_WRAP(function2, (int)(int)(int))
  MVP_WRAP_C(x, (int))
  MVP_WRAP_C(y, (int))
  MVP_WRAP(do_vector, (vw::Vector2)(vw::Vector3 const&))
END_MVP_WRAPPER()
