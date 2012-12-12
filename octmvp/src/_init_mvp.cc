#include <octave/oct.h>
#include <mvp/Octave/oct-mvpobj.h>

octave_value_list mvp_class(octave_value_list const& args, int nargout) {

  return octave_value(new octave_mvpobj_ref(new octave_mvpobj_impl<void>()));
}

DEFUN_DLD(_init_mvp, args, nargout, "register mvp classes") {
  octave_mvpobj_ref::register_type(); 

  install_builtin_function(mvp_class, "Stepper", std::string());
  install_builtin_function(mvp_class, "Seeder", std::string());
  install_builtin_function(mvp_class, "Correlator", std::string());
  install_builtin_function(mvp_class, "MvpClass", std::string());
  return octave_value();
}
