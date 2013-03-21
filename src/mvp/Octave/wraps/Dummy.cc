#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Algorithm/Dummy.h>
#include <mvp/Octave/OctaveWrapper.h>

BEGIN_OCTAVE_WRAPPER(Dummy, mvp::algorithm::Dummy, (int)(int)(mvp::algorithm::DummySettings const&))
  OCTAVE_WRAPPER(void0, (void))
  OCTAVE_WRAPPER(void1, (void)(int))
  OCTAVE_WRAPPER(void2, (void)(int)(int))
  OCTAVE_WRAPPER(function0, (int))
  OCTAVE_WRAPPER(function1, (int)(int))
  OCTAVE_WRAPPER(function2, (int)(int)(int))
  OCTAVE_WRAPPER_C(x, (int))
  OCTAVE_WRAPPER_C(y, (int))
  OCTAVE_WRAPPER(do_vector, (vw::Vector2)(vw::Vector3 const&))
END_OCTAVE_WRAPPER()

BEGIN_MVP_WRAPPER(Dummy, mvp::algorithm::Dummy, (int)(int)(mvp::algorithm::DummySettings const&))
  MVP_WRAPPER(void0, (void))
  MVP_WRAPPER(void1, (void)(int))
  MVP_WRAPPER(void2, (void)(int)(int))
  MVP_WRAPPER(function0, (int))
  MVP_WRAPPER(function1, (int)(int))
  MVP_WRAPPER(function2, (int)(int)(int))
  MVP_WRAPPER_C(x, (int))
  MVP_WRAPPER_C(y, (int))
  MVP_WRAPPER(do_vector, (vw::Vector2)(vw::Vector3 const&))
END_MVP_WRAPPER()
