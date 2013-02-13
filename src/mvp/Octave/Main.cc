#include <mvp/Octave/Main.h>

#include <octave/oct.h>
#include <octave/parse.h>
#include <octave/octave.h>
#include <octave/toplev.h>

#include <mvp/Octave/MvpWrapper.h>

///////////////////////////////////////////////////////////////////////
#include <mvp/Algorithm/Dummy.h>
#include <mvp/Octave/oct-mvpclass.h>

namespace mvp {
namespace octave {

/// algorithm -> octave
template <class ConstructT>
octave_value octave_wrap(algorithm::AlgoBase<ConstructT> const& v) {
  typedef typename algorithm::AlgoBase<ConstructT>::algorithm_type AlgoT;
  boost::shared_ptr<octave_mvpclass_base> ptr(new octave_mvpclass_wrap<AlgoT>(static_cast<AlgoT const&>(v)));
  return octave_value(new octave_mvpclass_ref(ptr));
}

}}

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


octave_value_list _new_mvpclass(octave_value_list const& args, int nargout) {
  return octave_value(new octave_mvpclass_ref(boost::shared_ptr<octave_mvpclass_base>(new octave_mvpclass_impl())));
}

// This should go in wraps/Dummy.cc somehow...
octave_value_list _create_Dummy(octave_value_list const& args, int nargout) {
  if (args.length() == 0) {
    error("No dummy type specified");
    return octave_value();
  }

  std::string name = args(0).string_value();

  return mvp::octave::octave_wrap(mvp::algorithm::Dummy(name, 5, 6));
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

namespace mvp {
namespace octave {

void start_octave_interpreter(std::string const& startup_script) {
  const char * argvv [] = {"", "--silent"};
  ::octave_main (2, (char **) argvv, true);
  if (!startup_script.empty()) {
    ::source_file(startup_script);
  }

  register_octave_mvp();
}

void register_octave_mvp() {
  octave_mvpclass_ref::register_type();

  install_builtin_function(_new_mvpclass, "mvpclass", std::string());

  install_builtin_function(_create_Dummy, "Dummy", std::string());
}

void stop_octave_interpreter() {
  do_octave_atexit();
}

}} // namespace mvp, octave
