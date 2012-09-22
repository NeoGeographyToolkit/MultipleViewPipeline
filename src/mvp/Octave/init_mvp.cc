#include <octave/oct.h> 

#include <mvp/Octave/ov-mvpobj-ref.h>

octave_value_list MvpSeeder(octave_value_list const& args, int nargout) {
  std::cout << "MvpSeeder" << std::endl;

  return octave_value(new octave_mvpobj_ref(new octave_mvpobj_base()));
}

DEFUN_DLD(init_mvp, args, nargout, "Load mvp functions/objects") {

  octave_mvpobj_ref::register_type(); 

  install_builtin_function(MvpSeeder, "MvpSeeder", std::string());

  return octave_value();
}
