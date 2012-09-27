/// \file ov-mvpobj-ref.h
///
/// mvpobj ref
///

#ifndef __MVP_OCTAVE_MVPOBJ_REF_H__
#define __MVP_OCTAVE_MVPOBJ_REF_H__

#include <octave/ov.h>

class octave_mvpobj_base;
class octave_typeinfo;

class octave_mvpobj_ref : public octave_base_value {
  octave_mvpobj_base *m_ptr;

  public:
    octave_mvpobj_ref() : m_ptr(NULL) {}

    octave_mvpobj_ref(octave_mvpobj_base *ptr);

    ~octave_mvpobj_ref();

    octave_mvpobj_base *ptr() { return m_ptr; }

    virtual octave_base_value *clone() const { return new octave_mvpobj_ref(m_ptr); }

    virtual bool is_defined() const { return true; }

    virtual octave_idx_type numel(octave_value_list const& idx) { return 1; }

    virtual octave_value
    subsref (std::string const& type, std::list<octave_value_list> const& idx);

    virtual octave_value_list 
    subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout);

    virtual octave_value
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs);

    virtual void print (std::ostream& os, bool pr_as_read_syntax = false) const;

  private:
    DECLARE_OCTAVE_ALLOCATOR;
    DECLARE_OV_TYPEID_FUNCTIONS_AND_DATA;

    // Prevent copy construct and assignment
    octave_mvpobj_ref(octave_mvpobj_ref const&);
    octave_mvpobj_ref& operator=(octave_mvpobj_ref const&);
};

#endif
