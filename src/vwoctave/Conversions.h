/// \file Conversions.h
///
/// Functions to interface octave and VW objects
///
/// TODO: Write something here

#ifndef __VWOCTAVE_CONVERSIONS_H__
#define __VWOCTAVE_CONVERSIONS_H__

#include <octave/oct.h>

#include <vw/FileIO/DiskImageResource.h>
#include <vw/Image/ImageView.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/GeoReference.h>

namespace vw {
namespace octave {

// TODO: Make a test for this too
::Matrix pinhole_to_octave(camera::PinholeModel const& vw_cam)
{
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

// TODO: Make a test for this function
template <class ViewT>
::Matrix imageview_to_octave(ImageViewBase<ViewT> const& vw_img) 
{
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

// TODO: Make a test here too
ImageView<double> octave_to_imageview(::Matrix const& oct_img)
{
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

}} // namespace vw, octave

#endif
