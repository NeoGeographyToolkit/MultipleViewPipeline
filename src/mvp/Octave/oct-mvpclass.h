/// \file ov-mvpclass.h
///
/// mvpclass
///

#ifndef __MVP_OCTAVE_MVPCLASS_H__
#define __MVP_OCTAVE_MVPCLASS_H__

#include <mvp/Octave/ov-mvpclass-ref.h>

#include <octave/oct-map.h>

#include <boost/utility.hpp>

template <class ClassT>
octave_value mvp_wrapper(ClassT *impl, std::string const& func, octave_value_list const& args);

struct octave_mvpclass_base : boost::noncopyable {
  virtual ~octave_mvpclass_base() {}

  virtual octave_value_list 
  subsref (octave_value const& parent_ref, std::string const& type, std::list<octave_value_list> const& idx, int nargout) = 0;

  virtual octave_value
  subsasgn (octave_value const& parent_ref, std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) = 0;

  virtual void
  print (std::ostream& os, bool pr_as_read_syntax = false) const = 0;
};

class octave_mvpclass_impl : public octave_mvpclass_base {
  octave_scalar_map m_map;
  
  public:
    virtual octave_value_list 
    subsref (octave_value const& parent_ref, std::string const& type, std::list<octave_value_list> const& idx, int nargout) {
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
              (++idx_copy.begin())->prepend(parent_ref);
            }

            retval = retval(0).next_subsref(nargout, type, idx_copy, 1);
          }
        } else {
          error("mvpclass_impl has no member `%s'", field.c_str());
        }
      } else {
        error("unsupported subsref for mvpclass_impl");
      }

      return retval;
    }

    virtual octave_value
    subsasgn (octave_value const& parent_ref, std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) { 
      if (idx.size() > 1) {
        std::list<octave_value_list> new_idx(idx.begin(), --idx.end());
        octave_value_list ovl = subsref(parent_ref, type.substr(0, type.size() - 1), new_idx, 1);
        if (ovl.length()) {
          ovl(0).subsasgn(type.substr(type.size() - 1), std::list<octave_value_list>(1, *--idx.end()), rhs);
        } else {
          error("mvpclass_impl subexpression did not return a result");
        }
      } else if (type[0] == '(' || type[0] == '{') {
        error("mvpclass_impl does not support indexing");
      } else if (type[0] == '.') {
        std::string field = idx.front()(0).string_value();
        m_map.setfield(field, rhs);
      } else {
        error("unsupported subsasgn for mvpclass_impl");
      }

      return parent_ref;
    }

    virtual void
    print (std::ostream& os, bool pr_as_read_syntax = false) const {
      os << "<<mvpclass_impl>>" << std::endl;
    }
};

template <class ClassT>
class octave_mvpclass_wrap : public octave_mvpclass_base {
  ClassT m_impl;

  public:
    octave_mvpclass_wrap(ClassT const& impl) : m_impl(impl) {}

    virtual octave_value_list 
    subsref (octave_value const& parent_ref, std::string const& type, std::list<octave_value_list> const& idx, int nargout) {
      octave_value_list retval;

      if (type[0] == '.') {
        assert(idx.front().length() == 1);

        std::string method = idx.front()(0).string_value();

        if (idx.size() > 1) {
          retval(0) = mvp_wrapper(&m_impl, method, *(++idx.begin()));

          if (idx.size() > 2) {
            retval = retval(0).next_subsref(nargout, type, idx, 2);
          }
        } else {
          error("mvpclass_wrap expected ()");
        }
      } else {
        error("unsupported subsref for mvpclass_impl");
      }

      return retval;
    }

    virtual octave_value
    subsasgn (octave_value const& parent_ref, std::string const& type, std::list<octave_value_list> const& idx, octave_value const& rhs) {
      error("subsasgn not supported for wrapped mvp classes");
      return parent_ref;
    }

    virtual void
    print (std::ostream& os, bool pr_as_read_syntax = false) const {
      os << "<<mvpclass_wrap>>" << std::endl;
    }

    ClassT impl() {
      return m_impl;
    } 
};

#endif
