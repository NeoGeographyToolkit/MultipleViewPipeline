#include <octave/oct.h>
#include <mvp/Octave/oct-mvpobj.h>

DEFUN_DLD(MvpClass, args, nargout, "Mvp Base Class") {
  octave_mvpobj_ref::register_type(); 
  return octave_value(new octave_mvpobj_ref(new octave_mvpobj_impl<void>()));
}
