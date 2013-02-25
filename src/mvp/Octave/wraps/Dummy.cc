#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Algorithm/Dummy.h>

BEGIN_MVP_WRAPPER(Dummy, mvp::algorithm::Dummy)
  MVP_WRAP_CONSTRUCTOR((std::string)(int)(int))
  MVP_WRAP_VOID(void0)
  MVP_WRAP_VOID(void1, (int))
  MVP_WRAP_VOID(void2, (int)(int))
  MVP_WRAP_FUNCTION(function0)
  MVP_WRAP_FUNCTION(function1, (int))
  MVP_WRAP_FUNCTION(function2, (int)(int))
  MVP_WRAP_FUNCTION(do_vector, (vw::Vector3))
END_MVP_WRAPPER()
