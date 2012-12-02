/// \file oct-cast.h
///
/// octave_cast function
///

#ifndef __MVP_OCTAVE_OCTCAST_H__
#define __MVP_OCTAVE_OCTCAST_H__

#include <octave/oct.h>
#include <octave/oct-map.h>

#include <vw/Math/Vector.h>
#include <vw/Math/Matrix.h>
#include <vw/Math/Quaternion.h>

namespace mvp {
namespace octave {

VW_DEFINE_EXCEPTION(BadCastErr, vw::Exception);

/// numeric type -> octave
template <class ToT, class FromT>
typename boost::enable_if<boost::is_arithmetic<FromT>, ToT>::type  
octave_cast(FromT const& v) {
  return ToT(double(v));
}

/// octave -> numeric type
template <class ToT>
typename boost::enable_if<boost::is_arithmetic<ToT>, ToT>::type
octave_cast(octave_value const& v) {
  VW_ASSERT(v.is_scalar_type(), BadCastErr() << "Not a scalar type");
  return ToT(v.double_value());
}

/// vw::vector -> octave
template <class ToT, class VectorT>
ToT octave_cast(vw::math::VectorBase<VectorT> const& v) {
  VectorT vw_vect(v.impl());
  ColumnVector oct_vect(vw_vect.size());

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    oct_vect(i) = vw_vect[i];
  }

  return oct_vect;
}

/// octave -> vw::vector
template <class VectorT>
typename boost::enable_if<boost::is_base_of<vw::math::VectorBase<VectorT>, VectorT>, VectorT>::type
octave_cast(octave_value const& v) {
  VW_ASSERT(v.is_matrix_type(), BadCastErr() << "Not a matrix type");

  VectorT vw_vect;
  ColumnVector oct_vect = v.column_vector_value();

  if (vw_vect.size() == 0) {
    vw_vect = VectorT(oct_vect.length());
  }

  VW_ASSERT(vw_vect.size() == unsigned(oct_vect.length()), BadCastErr() << "Bad vector size");

  for (int i = 0; i < oct_vect.length(); i++) {
    vw_vect[i] = oct_vect(i);
  }

  return vw_vect;
}

/// vw::quat -> octave
template <class ToT, class QuaternionT>
ToT octave_cast(vw::math::QuaternionBase<QuaternionT> const& q) {
  QuaternionT vw_quat(q.impl());
  ColumnVector oct_vect(4);

  for (unsigned i = 0; i < 4; i++) {
    oct_vect(i) = vw_quat[i];
  }

  return oct_vect;
}

/// octave -> vw::quat
template <class QuaternionT>
typename boost::enable_if<boost::is_base_of<vw::math::QuaternionBase<QuaternionT>, QuaternionT>, QuaternionT>::type
octave_cast(octave_value const& q) {
  return QuaternionT(octave_cast<vw::Vector<double, 4> >(q));
}

}} // namespace octave, mvp

#endif
