#ifndef __VWOCTAVE_CONVERSIONS_HH__
#define __VWOCTAVE_CONVERSIONS_HH__

#include <octave/lo-ieee.h> // octave_NA

namespace vw {
namespace octave {

template <class VectorT>
::ColumnVector vector_to_octave(VectorBase<VectorT> const& v) {
  VectorT vw_vect(v.impl());
  ::ColumnVector oct_vect(vw_vect.size());

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    oct_vect(i) = vw_vect[i];
  }

  return oct_vect;
}

template <class MatrixT>
::Matrix matrix_to_octave(MatrixBase<MatrixT> const& m) {
  MatrixT vw_mat(m.impl());
  ::Matrix oct_mat(vw_mat.rows(), vw_mat.cols());
  
  for (unsigned col = 0; col < vw_mat.cols(); col++) {
    for (unsigned row = 0; row < vw_mat.rows(); row++) {
      oct_mat(row, col) = vw_mat(col, row);
    }
  }

  return oct_mat;
}

template <class ViewT>
::Matrix imageview_to_octave(ImageViewBase<ViewT> const& vw_img) {
  typedef ImageView<PixelMask<double> > RasterT;

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

}} // namespace octave, vw

#endif
