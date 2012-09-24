/// \file ov-mvpobj.h
///
/// mvpobj
///

#ifndef __MVP_OCTAVE_MVPOBJ_H__
#define __MVP_OCTAVE_MVPOBJ_H__

#include <mvp/Octave/ov-mvpobj-ref.h>

#include <octave/oct-map.h>

#include <boost/utility.hpp>

namespace mvp {
namespace wrapper {

template <class ObjT>
octave_value mvp_wrapper(ObjT *obj, std::string const& func, octave_value_list const& args);

}} // namespace wrapper, mvp

class octave_mvpobj_base : boost::noncopyable {
  int refcount;

  protected:
    octave_value as_value() {
      return octave_value(new octave_mvpobj_ref(this));
    }

  public:
    friend class octave_mvpobj_ref;

    octave_mvpobj_base() : refcount(0) {}

    virtual ~octave_mvpobj_base() {}

    virtual octave_value_list 
    subsref (std::string const& type, std::list<octave_value_list> const& idx, int nargout) = 0;

    virtual octave_value
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
        assert(idx.front().length() == 1);

        std::string field = idx.front()(0).string_value();

        if (m_map.contains(field)) {
          retval(0) = m_map.getfield(field);
          
          if (idx.size() > 1) {
            std::list<octave_value_list> idx_copy = idx;

            // Add 'self' to member functions
            if (retval(0).is_function_handle() && type[1] == '(') {
              (++idx_copy.begin())->prepend(this->as_value());
            }

            retval = retval(0).next_subsref(nargout, type, idx_copy, 1);
          }
        } else {
          error("mvpobj_impl has no member `%s'", field.c_str());
        }
      } else {
        error("unsupported subsref for mvpobj_impl");
      }

      return retval;
    }

    virtual octave_value
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) { 
      if (idx.size() > 1) {
        std::list<octave_value_list> new_idx(idx.begin(), --idx.end());
        octave_value_list ovl = subsref(type.substr(0, type.size() - 1), new_idx, 1);
        if (ovl.length()) {
          ovl(0).subsasgn(type.substr(type.size() - 1), std::list<octave_value_list>(1, *--idx.end()), rhs);
        } else {
          error("mvpobj_impl subexpression did not return a result");
        }
      } else if (type[0] == '(' || type[0] == '{') {
        error("mvpobj_impl does not support indexing");
      } else if (type[0] == '.') {
        std::string field = idx.front()(0).string_value();
        m_map.setfield(field, rhs);
      } else {
        error("unsupported subsasgn for mvpobj_impl");
      }

      return this->as_value();
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
      octave_value_list retval;

      if (type[0] == '.') {
        assert(idx.front().length() == 1);

        std::string method = idx.front()(0).string_value();

        if (idx.size() > 1) {
          retval(0) = mvp::wrapper::mvp_wrapper(m_obj, method, *(++idx.begin()));

          if (idx.size() > 2) {
            retval = retval(0).next_subsref(nargout, type, idx, 2);
          }
        } else {
          error("mvpobj_wrap expected ()");
        }
      } else {
        error("unsupported subsref for mvpobj_impl");
      }

      return retval;
    }

    virtual octave_value
    subsasgn (std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) {
      error("subsasgn not supported for wrapped mvp classes");
      return this->as_value();
    }

    virtual ObjectT *obj() {
      return m_obj;
    } 
};

#endif
