#include <octave/config.h>
#include <octave/ov-typeinfo.h>

#include <mvp/Octave/ov-mvpclass-ref.h>
#include <mvp/Octave/oct-mvpclass.h>


DEFINE_OCTAVE_ALLOCATOR(octave_mvpclass_ref);
DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(octave_mvpclass_ref, "mvpclass_ref", "mvpclass_ref");

octave_value
octave_mvpclass_ref::subsref (std::string const& type, std::list<octave_value_list> const& idx) {
  octave_value_list ovl = subsref(type, idx, 1);
  return ovl.length() ? ovl(0) : octave_value(); 
}

octave_value_list 
octave_mvpclass_ref::subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) {
  if (m_ptr) {
    return m_ptr->subsref(octave_value(this, true), type, idx, nargout); 
  } else {
    error("mvpclass_ref has NULL pointer");
    return octave_value();
  }
}

octave_value
octave_mvpclass_ref::subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) { 
  if (m_ptr) {
    return m_ptr->subsasgn(octave_value(this, true), type, idx, rhs); 
  } else {
    error("mvpclass_ref has NULL pointer");
    return octave_value(this);
  }
}

void octave_mvpclass_ref::print (std::ostream& os, bool pr_as_read_syntax) const {
  if (m_ptr) {
    return m_ptr->print(os, pr_as_read_syntax);
  } else {
    error("mvpclass_ref has NULL pointer");
  }
}
