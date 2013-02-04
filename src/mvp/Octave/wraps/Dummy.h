/// \file Dummy.h
///
/// TODO: Write doc
///

#ifndef __MVP_OCTAVE_DUMMY_H__
#define __MVP_OCTAVE_DUMMY_H__


#include <mvp/Algorithm/Dummy/DerivedDummy.h>
#include <mvp/Octave/oct-mvpclass.h>

#include <mvp/Octave/Conversions.h>

octave_value_list _new_mvpclass(octave_value_list const& args, int nargout) {
  return octave_value(new octave_mvpclass_ref(boost::shared_ptr<octave_mvpclass_base>(new octave_mvpclass_impl())));
}


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

template <>
octave_value mvp_wrapper<mvp::algorithm::Dummy>(mvp::algorithm::Dummy const& impl, std::string const& func, octave_value_list const& args);

octave_value_list _create_Dummy(octave_value_list const& args, int nargout) {
  if (args.length() == 0) {
    error("No dummy type specified");
    return octave_value();
  }

  std::string name = args(0).string_value();

  return mvp::octave::octave_wrap(mvp::algorithm::Dummy(name, 5, 6));
}

template <>
octave_value mvp_wrapper<mvp::algorithm::Dummy>(mvp::algorithm::Dummy const& impl, std::string const& func, octave_value_list const& args) {
  
  std::cout << func << std::endl;

  return octave_value(100);
}

#endif
