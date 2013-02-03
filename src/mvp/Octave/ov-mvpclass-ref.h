/// \file ov-mvpclass-ref.h
///
/// mvpclass ref
///

#ifndef __MVP_OCTAVE_MVPCLASS_REF_H__
#define __MVP_OCTAVE_MVPCLASS_REF_H__

#include <octave/ov.h>
#include <boost/shared_ptr.hpp>

class octave_mvpclass_base;
class octave_typeinfo;

class octave_mvpclass_ref : public octave_base_value {
  boost::shared_ptr<octave_mvpclass_base> m_ptr;

  public:
    octave_mvpclass_ref() : m_ptr() {}

    octave_mvpclass_ref(boost::shared_ptr<octave_mvpclass_base> ptr) : m_ptr(ptr) {}

    boost::shared_ptr<octave_mvpclass_base> ptr() { return m_ptr; }

    virtual octave_base_value *clone() const { return new octave_mvpclass_ref(m_ptr); }

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
    octave_mvpclass_ref(octave_mvpclass_ref const&);
    octave_mvpclass_ref& operator=(octave_mvpclass_ref const&);
};

#endif
