/// \file ov-mvpobj.h
///
/// mvpobj
///

#ifndef __MVP_OCTAVE_MVPOBJ_H__
#define __MVP_OCTAVE_MVPOBJ_H__

#include <octave/ov.h>

class octave_mvpobj_base {

  public:
    virtual octave_value_list 
    subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) {
      std::cout << "subsref" << std::endl;
      return octave_value();
    }

    virtual void
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) {
      std::cout << "subsasgn" << std::endl;
    }
};

template <class ObjectT>
class octave_mvpobj : public octave_mvpobj_base {
  ObjectT *m_obj;

  public:
    ObjectT *obj() { return m_obj; }
};

template <class ObjectT>
class octave_mvpobj_impl : public octave_mvpobj<ObjectT> {
  // has default constructor
  // implement subsref
};

template <class ObjectT>
class octave_mvpobj_wrap : public octave_mvpobj<ObjectT> {
  // has constructor with ObjectT
  // implement subsref
};

#endif
