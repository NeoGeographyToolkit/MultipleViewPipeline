#include <octave/oct.h> 

#include <mvp/Octave/Main.h>

DEFUN_DLD(init_mvp, args, nargout, "Load mvp functions/objects") {

  mvp::octave::register_octave_mvp();

  return octave_value();
}
