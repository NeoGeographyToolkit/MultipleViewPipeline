#include <octave/oct.h>
#include <mvp/Octave/oct-mvpclass.h>

octave_value_list mvp_class(octave_value_list const& args, int nargout) {
  return octave_value(new octave_mvpclass_ref(boost::shared_ptr<octave_mvpclass_base>(new octave_mvpclass_impl())));
}

DEFUN_DLD(_init_mvp, args, nargout, "register mvp classes") {
  octave_mvpclass_ref::register_type(); 
  install_builtin_function(mvp_class, "mvpclass", std::string());
  return octave_value();
}
