/// \file ov-mvpobj.h
///
/// mvpobj
///

#ifndef __MVP_OCTAVE_MVPOBJ_H__
#define __MVP_OCTAVE_MVPOBJ_H__

#include <octave/ov.h>
#include <octave/ov-struct.h>

#include <boost/utility.hpp>

class octave_mvpobj_base : boost::noncopyable {
  public:
    virtual ~octave_mvpobj_base() {}

    virtual octave_value_list 
    subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) = 0;

    virtual void
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) = 0;
};

template <class ObjectT>
class octave_mvpobj : public octave_mvpobj_base {
  public:
    virtual ObjectT *obj() = 0;
};

template <class ObjectT>
class octave_mvpobj_impl : public octave_mvpobj<ObjectT> {
  octave_scalar_map m_map;
  
  public:
    virtual octave_value_list 
    subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) {
      octave_value_list retval;

      if (type[0] == '.') {
        int skip = 1;

        assert(idx.front().length() == 1);

        std::string field = idx.front()(0).string_value();

        if (m_map.contains(field)) {
          retval(0) = m_map.getfield(field);
          
          if (idx.size () > 1) {
            // if next type is a (, and retval(0) is a function handle prepend self to idx(1)
            retval = retval(0).next_subsref(nargout, type, idx, skip);
          }
        } else {
          error("mvpobj_impl has no member `%s'", field.c_str());
        }
      } else {
        error("invalid index for mvpobj_impl");
      }

      return retval;
    }

    virtual void
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) { 
      if (idx.size() > 1) {
        error("FIXME: Nested subsasgn unsupported for now");
      } {
        m_map.setfield("asdf", 5);
        //m_impl.subsasgn(type, idx, rhs); 
      }
    }

    virtual ObjectT *obj() {
      return NULL;
    }
};

template <class ObjectT>
class octave_mvpobj_wrap : public octave_mvpobj<ObjectT> {
  ObjectT *m_obj;
  bool m_owns_obj;

  public:
    octave_mvpobj_wrap(ObjectT *obj, bool owns_obj = false) : m_obj(obj), m_owns_obj(owns_obj) {}

    virtual ~octave_mvpobj_wrap() {
      if (m_owns_obj) {
        delete m_obj;
      }
    }

    virtual octave_value_list 
    subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) {
      std::cout << "subsref" << std::endl;
      return octave_value();
    }

    virtual void
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) {
      error("subsasgn not supported for wrapped mvp classes");
    }

    virtual ObjectT *obj() {
      return m_obj;
    } 
};

#endif
