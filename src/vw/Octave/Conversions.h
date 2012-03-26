/// \file Conversions.h
///
/// Functions to interface octave and VW objects
///

#ifndef __VWOCTAVE_CONVERSIONS_H__
#define __VWOCTAVE_CONVERSIONS_H__

#include <octave/oct.h>
#include <octave/oct-map.h>

#include <vw/FileIO/DiskImageResource.h>
#include <vw/Image/ImageView.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/GeoReference.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

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
inline Vector<double, 0> octave_to_vector(::ColumnVector const& oct_vect) {
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
inline Matrix<double, 0> octave_to_matrix(::Matrix const& oct_mat) {
  Matrix<double, 0> vw_mat(oct_mat.cols(), oct_mat.rows());

  for (unsigned col = 0; col < vw_mat.cols(); col++) {
    for (unsigned row = 0; row < vw_mat.rows(); row++) {
      vw_mat(col, row) = oct_mat(row, col);
    }
  }

  return vw_mat;
}

/// Convert a VW PinholeModel to an Octave Matrix
inline ::Matrix pinhole_to_octave(camera::PinholeModel const& vw_cam) {
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
  typedef ImageView<PixelMask<double> > RasterT;

  // Rasterize image before copying to octave
  RasterT rast = vw_img.impl();

  ::Matrix oct_img(rast.rows(), rast.cols());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      oct_img(row, col) = is_valid(*cacc) ? remove_mask(*cacc) : std::numeric_limits<double>::quiet_NaN();
      cacc.next_col();
    }
    racc.next_row();
  }

  return oct_img;
}

/// Convert an Octave Matrix to a VW ImageView
inline ImageView<PixelMask<double> > octave_to_imageview(::Matrix const& oct_img) {
  typedef ImageView<PixelMask<double> > RasterT;

  RasterT rast(oct_img.cols(), oct_img.rows());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      *cacc = std::isnan(oct_img(row, col)) ? PixelMask<double>() : PixelMask<double>(oct_img(row, col));
      cacc.next_col();
    }
    racc.next_row();
  }

  return rast;
}

/// Convert a VW GeoReference to an Octave Matrix
inline ::octave_scalar_map georef_to_octave(cartography::GeoReference const& vw_geo) {
  if (vw_geo.is_projected()) {
    vw_throw(ArgumentErr() << "Projected georefs not supported!");
  }

  Matrix3x3 vw_geo_trans = vw_geo.transform();
  ::Matrix oct_geo_trans(3, 3);

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 3; c++) {
      oct_geo_trans(r, c) = vw_geo_trans(r, c);
    }
  }

  ::Matrix rebase = ::identity_matrix(3, 3);
  if (vw_geo.pixel_interpretation() == cartography::GeoReference::PixelAsPoint) {
    rebase(0, 2) = rebase(1, 2) = -1;
  } else {
    rebase(0, 2) = rebase(1, 2) = -0.5;
  }
  oct_geo_trans = oct_geo_trans * rebase;

  ::Matrix deg2rad = ::identity_matrix(3, 3);
  deg2rad(0, 0) = deg2rad(1, 1) = M_PI / 180.0;
  oct_geo_trans = deg2rad * oct_geo_trans;

  ::octave_scalar_map oct_datum;
  oct_datum.setfield("semi_major_axis", vw_geo.datum().semi_major_axis());
  oct_datum.setfield("semi_minor_axis", vw_geo.datum().semi_minor_axis());

  ::octave_scalar_map result;
  result.setfield("transform", oct_geo_trans);
  result.setfield("datum", oct_datum);

  return result;
}

inline ::octave_scalar_map protobuf_to_octave(const google::protobuf::Message *message) {
  using namespace google::protobuf;

  ::octave_scalar_map result;

  const Descriptor *descriptor = message->GetDescriptor();
  const Reflection *reflection = message->GetReflection();

  for (int i = 0; i < descriptor->field_count(); i++) {
    const FieldDescriptor *field = descriptor->field(i);

    const std::string field_name(field->name());

    switch(field->type()) {
      case FieldDescriptor::TYPE_DOUBLE:
        result.setfield(field_name, reflection->GetDouble(*message, field));
        break;
      case FieldDescriptor::TYPE_FLOAT:
        result.setfield(field_name, reflection->GetFloat(*message, field));
        break;
      case FieldDescriptor::TYPE_INT32:
        result.setfield(field_name, reflection->GetInt32(*message, field));
        break;
      case FieldDescriptor::TYPE_BOOL:
        result.setfield(field_name, reflection->GetBool(*message, field));
        break;
      case FieldDescriptor::TYPE_STRING:
        result.setfield(field_name, reflection->GetString(*message, field));
        break;
      default:
        vw::vw_throw(vw::NoImplErr() << "Not a supported field type to convert to octave");
    }
  }

  return result;
}

}} // namespace vw, octave

#endif
