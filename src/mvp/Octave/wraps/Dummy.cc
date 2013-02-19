#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Algorithm/Dummy.h>

/*

/// octave -> algorithm::Dummy
template <class DummyT>
typename boost::enable_if<boost::is_same<algorithm::Dummy, DummyT>, DummyT>::type
octave_as(octave_value const& v) {
  octave_mvpclass_ref *ref = dynamic_cast<octave_mvpclass_ref*>(v.internal_rep());
  VW_ASSERT(ref, BadCastErr() << "Not an mvpclass");

  boost::shared_ptr<octave_mvpclass_wrap<algorithm::Dummy> > wrap = dynamic_pointer_cast<octave_mvpclass_wrap<algorithm::Dummy> >(ref->ptr())
  if (wrap) {
    return wrap->impl();
  } else {
    // Return an OctaveDummy    
    return algorithm::Dummy("DerivedDummy");
  }  
}

}} // namespace octave,mvp
*/

octave_value_list _new_Dummy(octave_value_list const& args, int nargout) {
  if (args.length() == 0) {
    error("No dummy type specified");
    return octave_value();
  }

  std::string name = args(0).string_value();

  return mvp::octave::octave_wrap(mvp::algorithm::Dummy(name, 5, 6));
}

class k_Helper {
  static MvpWrapperInstallerRegistrar reg;
};

MvpWrapperInstallerRegistrar k_Helper::reg(_new_Dummy, "Dummy", std::string());

BEGIN_MVP_WRAPPER(mvp::algorithm::Dummy)
  MVP_WRAP(void0)
  MVP_WRAP(void1)
  MVP_WRAP(void2)
  MVP_WRAP(function0)
  MVP_WRAP(function1)
  MVP_WRAP(function2)
//  MVP_WRAP(do_vector)
END_MVP_WRAPPER()
