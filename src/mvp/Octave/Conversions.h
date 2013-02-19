/// \file oct-cast.h
///
/// octave_as function
///

#ifndef __MVP_OCTAVE_OCTCAST_H__
#define __MVP_OCTAVE_OCTCAST_H__

#include <mvp/Octave/oct-mvpclass.h>

#include <vw/Core/Exception.h>

namespace mvp {
namespace octave {

VW_DEFINE_EXCEPTION(BadCastErr, vw::Exception);

/// algorithm -> octave
template <class T>
octave_value octave_wrap(T const& v) {
  boost::shared_ptr<octave_mvpclass_base> ptr(new octave_mvpclass_wrap<T>(v));
  return octave_value(new octave_mvpclass_ref(ptr));
}

template <class T>
T octave_as(octave_value const& v) {

  return T();
}

template <>
octave_value octave_wrap<int>(int const& v);


}} // namespace octave, mvp

#endif
