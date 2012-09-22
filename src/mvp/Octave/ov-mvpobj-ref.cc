#include <octave/config.h>
#include <octave/ov-typeinfo.h>

#include <mvp/Octave/ov-mvpobj-ref.h>
#include <mvp/Octave/oct-mvpobj.h>


DEFINE_OCTAVE_ALLOCATOR(octave_mvpobj_ref);
DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(octave_mvpobj_ref, "mvpobj_ref", "mvpobj_ref");

octave_mvpobj_ref::octave_mvpobj_ref(octave_mvpobj_base *ptr) : m_ptr(ptr) {m_ptr->refcount++;}

octave_mvpobj_ref::~octave_mvpobj_ref() {
  if (m_ptr) {
    if (!--m_ptr->refcount) {
      delete m_ptr;
    } 
  }
}

octave_value
octave_mvpobj_ref::subsref (std::string const& type, std::list<octave_value_list> const& idx) {
  octave_value_list ovl = subsref(type, idx, 1);
  return ovl.length() ? ovl(0) : octave_value(); 
}

octave_value_list 
octave_mvpobj_ref::subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) {
  if (m_ptr) {
    return m_ptr->subsref(type, idx, nargout); 
  } else {
    error("mvpobj_ref has NULL pointer");
    return octave_value();
  }
}

octave_value
octave_mvpobj_ref::subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) { 
  if (m_ptr) {
    return m_ptr->subsasgn(type, idx, rhs); 
  } else {
    error("mvpobj_ref has NULL pointer");
    return octave_value();
  }
}
