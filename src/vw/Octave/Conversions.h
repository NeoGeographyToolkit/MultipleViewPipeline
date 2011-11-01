/// \file Conversions.h
///
/// Functions to interface octave and VW objects
///

#ifndef __VWOCTAVE_CONVERSIONS_H__
#define __VWOCTAVE_CONVERSIONS_H__

#include <octave/oct.h>

#include <vw/FileIO/DiskImageResource.h>
#include <vw/Image/ImageView.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/GeoReference.h>

namespace vw {
namespace octave {

/// Convert a VW Vector to an Octave ColumnVector
template <class VectorT>
::ColumnVector vector_to_octave(VectorBase<VectorT> const& v) {
  VectorT vw_vect(v.impl());
  ::ColumnVector oct_vect(vw_vect.size());

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    oct_vect(i) = vw_vect[i];
  }

  return oct_vect;
}

/// Convert an Octave ColumnVector to a VW Vector
Vector<double, 0> octave_to_vector(::ColumnVector const& oct_vect) {
  Vector<double, 0> vw_vect(oct_vect.length());

  for (int i = 0; i < oct_vect.length(); i++) {
    vw_vect[i] = oct_vect(i);
  }

  return vw_vect;
}

/// Convert a VW Matrix to an Octave Matrix
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

/// Convert an Octave Matrix into a VW Matrix
Matrix<double, 0> octave_to_matrix(::Matrix const& oct_mat) {
  Matrix<double, 0> vw_mat(oct_mat.cols(), oct_mat.rows());

  for (unsigned col = 0; col < vw_mat.cols(); col++) {
    for (unsigned row = 0; row < vw_mat.rows(); row++) {
      vw_mat(col, row) = oct_mat(row, col);
    }
  }

  return vw_mat;
}

/// Convert a VW PinholeModel to an Octave Matrix
::Matrix pinhole_to_octave(camera::PinholeModel const& vw_cam) {
  ::Matrix oct_cam_mat(3, 4);

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 4; c++) {
      oct_cam_mat(r, c) = vw_cam.camera_matrix()(r, c);
    }
  }

  ::Matrix rebase = ::identity_matrix(3, 3);
  rebase(0, 2) = rebase(1, 2) = 1;
  oct_cam_mat = rebase * oct_cam_mat;

  return oct_cam_mat;
}

/// Convert a VW ImageView to an Octave Matrix
template <class ViewT>
::Matrix imageview_to_octave(ImageViewBase<ViewT> const& vw_img) {
  typedef ImageView<double> RasterT;

  // Rasterize image before copying to octave
  RasterT rast = pixel_cast<double>(vw_img.impl());

  ::Matrix oct_img(rast.rows(), rast.cols());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      oct_img(row, col) = *cacc;
      cacc.next_col();
    }
    racc.next_row();
  }

  return oct_img;
}

/// Convert an Octave Matrix to a VW ImageView
ImageView<double> octave_to_imageview(::Matrix const& oct_img) {
  typedef ImageView<double> RasterT;

  RasterT rast(oct_img.cols(), oct_img.rows());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      *cacc = oct_img(row, col);
      cacc.next_col();
    }
    racc.next_row();
  }

  return rast;
}

/// Convert a VW GeoReference to an Octave Matrix
::Matrix georef_to_octave(cartography::GeoReference const& vw_geo) {
  if (vw_geo.is_projected()) {
    vw_throw(ArgumentErr() << "Projected georefs not supported!");
  }

  Matrix3x3 vw_geo_trans = vw_geo.transform();
  ::Matrix oct_geo(3, 3);

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      oct_geo(r, c) = vw_geo_trans(r, c);
    }
  }

  ::Matrix rebase = ::identity_matrix(3, 3);
  if (vw_geo.pixel_interpretation() == cartography::GeoReference::PixelAsPoint) {
    rebase(0, 2) = rebase(1, 2) = -1;
  } else {
    rebase(0, 2) = rebase(1, 2) = -0.5;
  }
  oct_geo = oct_geo * rebase;

  ::Matrix deg2rad = ::identity_matrix(3, 3);
  deg2rad(0, 0) = deg2rad(1, 1) = M_PI / 180.0;
  oct_geo = deg2rad * oct_geo;

  return oct_geo;
}

}} // namespace vw, octave

#endif
