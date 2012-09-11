/// \file Conversions.h
///
/// Functions to interface octave and VW objects
///

#ifndef __VWOCTAVE_CONVERSIONS_H__
#define __VWOCTAVE_CONVERSIONS_H__

#include <octave/oct.h>
#include <octave/oct-map.h>

#include <vw/Image/ImageView.h>

// TODO: These will go
#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/GeoReference.h>

namespace google {
namespace protobuf {
  class Message;
}} // namespace protobuf, google

namespace vw {
namespace octave {

/// Convert a VW Vector to an Octave ColumnVector
template <class VectorT>
::ColumnVector vector_to_octave(VectorBase<VectorT> const& v);

/// Convert a VW Matrix to an Octave Matrix
template <class MatrixT>
::Matrix matrix_to_octave(MatrixBase<MatrixT> const& m);

/// Convert a VW ImageView to an Octave Matrix
template <class ViewT>
::Matrix imageview_to_octave(ImageViewBase<ViewT> const& vw_img);

/// Convert an Octave ColumnVector to a VW Vector
Vector<double, 0> octave_to_vector(::ColumnVector const& oct_vect);

/// Convert an Octave Matrix into a VW Matrix
Matrix<double, 0> octave_to_matrix(::Matrix const& oct_mat);

/// Convert an Octave Matrix to a VW ImageView
ImageView<PixelMask<double> > octave_to_imageview(::Matrix const& oct_img);

/// Convert a protobuffer message to an octave map
::octave_scalar_map protobuf_to_octave(const google::protobuf::Message *message);

// TODO: Wrapper (use ptr!)
/// Convert a VW PinholeModel to an Octave Matrix
::Matrix pinhole_to_octave(camera::PinholeModel const& vw_cam); 

// TODO: Wrapper (use ptr!)
/// Convert a VW GeoReference to an Octave Matrix
::octave_scalar_map georef_to_octave(cartography::GeoReference const& vw_geo);

}} // namespace vw, octave

#include <vw/Octave/Conversions.hh>

#endif
