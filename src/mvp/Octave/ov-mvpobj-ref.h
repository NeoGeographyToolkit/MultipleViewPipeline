/// \file ov-mvpobj-ref.h
///
/// mvpobj ref
///

#ifndef __MVP_OCTAVE_MVPOBJ_REF_H__
#define __MVP_OCTAVE_MVPOBJ_REF_H__

#include <octave/ov.h>
#include <boost/shared_ptr.hpp>

#include <mvp/Octave/oct-mvpobj.h>

class octave_mvpobj_ref : public octave_base_value {
  boost::shared_ptr<octave_mvpobj_base> m_ptr;

  public:
    octave_mvpobj_ref() {}

    octave_mvpobj_ref(octave_mvpobj_base *ptr) : m_ptr(ptr) {}

    octave_mvpobj_ref(boost::shared_ptr<octave_mvpobj_base> ptr) : m_ptr(ptr) {}

    octave_mvpobj_base *ptr() { return m_ptr.get(); }

    virtual octave_base_value *clone() const { return new octave_mvpobj_ref(m_ptr); }

    virtual bool is_defined() const { return true; }

    virtual octave_idx_type numel(octave_value_list const& idx) { return 1; }

    virtual octave_value_list 
    subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) 
    { return m_ptr->subsref(type, idx, nargout); }

    virtual octave_value
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs)
    { m_ptr->subsasgn(type, idx, rhs); 
      return octave_value(clone()); }

  private:
    DECLARE_OCTAVE_ALLOCATOR;
    DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA;
};

DEFINE_OCTAVE_ALLOCATOR(octave_mvpobj_ref);
DEFINE_OV_TYPEID_FUNCTIONS_AND_DATA(octave_mvpobj_ref, "mvpobj_ref", "mvpobj_ref");

#endif
