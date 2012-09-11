#include <vw/Octave/Conversions.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

namespace vw {
namespace octave {

Vector<double, 0> octave_to_vector(::ColumnVector const& oct_vect) {
  Vector<double, 0> vw_vect(oct_vect.length());

  for (int i = 0; i < oct_vect.length(); i++) {
    vw_vect[i] = oct_vect(i);
  }

  return vw_vect;
}

Matrix<double, 0> octave_to_matrix(::Matrix const& oct_mat) {
  Matrix<double, 0> vw_mat(oct_mat.cols(), oct_mat.rows());

  for (unsigned col = 0; col < vw_mat.cols(); col++) {
    for (unsigned row = 0; row < vw_mat.rows(); row++) {
      vw_mat(col, row) = oct_mat(row, col);
    }
  }

  return vw_mat;
}

ImageView<PixelMask<double> > octave_to_imageview(::Matrix const& oct_img) {
  typedef ImageView<PixelMask<double> > RasterT;

  RasterT rast(oct_img.cols(), oct_img.rows());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      *cacc = ::xisnan(oct_img(row, col)) ? PixelMask<double>() : PixelMask<double>(oct_img(row, col));
      cacc.next_col();
    }
    racc.next_row();
  }

  return rast;
}

::octave_scalar_map protobuf_to_octave(const google::protobuf::Message *message) {
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
        ::vw::vw_throw(::vw::NoImplErr() << "Not a supported field type to convert to octave");
    }
  }

  return result;
}

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

::octave_scalar_map georef_to_octave(cartography::GeoReference const& vw_geo) {
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

}} // namespace octave, vw
