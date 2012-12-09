/// \file oct-cast.h
///
/// octave_cast function
///

#ifndef __MVP_OCTAVE_OCTCAST_H__
#define __MVP_OCTAVE_OCTCAST_H__

#include <octave/oct.h>
#include <octave/oct-map.h>
#include <octave/lo-ieee.h> // octave_NA

#include <vw/Math/Vector.h>
#include <vw/Math/Matrix.h>
#include <vw/Math/Quaternion.h>
#include <vw/Image/ImageView.h>
#include <vw/Image/PixelMask.h>

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

/// vw::quat -> octave
template <class QuaternionT>
typename boost::enable_if<boost::is_base_of<vw::math::QuaternionBase<QuaternionT>, QuaternionT>, QuaternionT>::type
octave_cast(octave_value const& q) {
  return QuaternionT(octave_cast<vw::Vector<double, 4> >(q));
}

/// vw::Matrix -> octave
template <class ToT, class MatrixT>
ToT octave_cast(vw::math::MatrixBase<MatrixT> const& m) {
  MatrixT vw_mat(m.impl());
  ::Matrix oct_mat(vw_mat.rows(), vw_mat.cols());
  
  for (unsigned col = 0; col < vw_mat.cols(); col++) {
    for (unsigned row = 0; row < vw_mat.rows(); row++) {
      oct_mat(row, col) = vw_mat(row, col);
    }
  }

  return oct_mat;
}

/// octave -> vw::Matrix
template <class MatrixT>
typename boost::enable_if<boost::is_base_of<vw::math::MatrixBase<MatrixT>, MatrixT>, MatrixT>::type
octave_cast(octave_value const& m) {
  VW_ASSERT(m.is_matrix_type(), BadCastErr() << "Not a matrix type");

  MatrixT vw_mat;
  ::Matrix oct_mat = m.matrix_value();

  if (vw_mat.rows() == 0 || vw_mat.cols() == 0) {
    vw_mat = MatrixT(oct_mat.rows(), oct_mat.cols());
  }

  VW_ASSERT(vw_mat.cols() == unsigned(oct_mat.cols()), BadCastErr() << "Bad matrix size");
  VW_ASSERT(vw_mat.rows() == unsigned(oct_mat.rows()), BadCastErr() << "Bad matrix size");

  for (unsigned col = 0; col < vw_mat.cols(); col++) {
    for (unsigned row = 0; row < vw_mat.rows(); row++) {
      vw_mat(row, col) = oct_mat(row, col);
    }
  }

  return vw_mat;
}

/// octave -> vw::imageview
template <class ToT, class ViewT>
ToT octave_cast(vw::ImageViewBase<ViewT> const& vw_img) {
  typedef vw::ImageView<vw::PixelMask<double> > RasterT;

  // Rasterize image before copying to octave
  RasterT rast = vw_img.impl();

  ::Matrix oct_img(rast.rows(), rast.cols());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      oct_img(row, col) = is_valid(*cacc) ? remove_mask(*cacc) : ::octave_NA;
      cacc.next_col();
    }
    racc.next_row();
  }

  return oct_img;
}

/// octave -> vw::imageview
template <class ViewT>
typename boost::enable_if<boost::is_base_of<vw::ImageViewBase<ViewT>, ViewT>, ViewT>::type
octave_cast(octave_value const& v) {
  VW_ASSERT(v.is_matrix_type(), BadCastErr() << "Not a matrix type");
  typedef vw::ImageView<vw::PixelMask<double> > RasterT;

  ::Matrix oct_img = v.matrix_value();
  RasterT rast(oct_img.cols(), oct_img.rows());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      *cacc = ::xisnan(oct_img(row, col)) ? vw::PixelMask<double>() : vw::PixelMask<double>(oct_img(row, col));
      cacc.next_col();
    }
    racc.next_row();
  }

  return rast;
}

}} // namespace octave, mvp

#endif
