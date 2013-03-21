/// \file oct-cast.h
///
/// Conversions to/from octave
///

#ifndef __MVP_OCTAVE_OCTCAST_H__
#define __MVP_OCTAVE_OCTCAST_H__

#include <mvp/Octave/oct-mvpclass.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <octave/lo-ieee.h> // octave_NA

#include <vw/Math/Vector.h>
#include <vw/Math/Matrix.h>
#include <vw/Math/Quaternion.h>
#include <vw/Image/ImageView.h>
#include <vw/Image/PixelMask.h>

#include <boost/preprocessor/seq/for_each.hpp>

namespace mvp {
namespace octave {

VW_DEFINE_EXCEPTION(BadCastErr, vw::Exception);

template <class T, class Enable=void>
struct ConversionHelper {
  static octave_value to_octave(T const& v) {
    boost::shared_ptr<octave_mvpclass_base> ptr(new octave_mvpclass_wrap<T>(v));
    return octave_value(new octave_mvpclass_ref(ptr));
  }
  static T from_octave(octave_value const& v) {
    octave_mvpclass_ref *ref = dynamic_cast<octave_mvpclass_ref*>(v.internal_rep());
    VW_ASSERT(ref, BadCastErr() << "Not an mvpclass");
    boost::shared_ptr<octave_mvpclass_wrap<T> > wrap = boost::dynamic_pointer_cast<octave_mvpclass_wrap<T> >(ref->ptr());
    if (wrap) {
      return wrap->impl();
    } else {
      return octave_wrapper<T>(v);
    }
  }
};

template <class T>
octave_value to_octave(T const& v) {
  return ConversionHelper<T>::to_octave(v);
}

template <class T>
T from_octave(octave_value const& v) {
  typedef typename boost::remove_cv<typename boost::remove_reference<T>::type>::type T_stripped;
  return ConversionHelper<T_stripped>::from_octave(v);
}

/// Specializations...

/// allow void to_octave for OctaveWrappers
template <>
struct ConversionHelper<void> {
  static void from_octave(octave_value const& v) {}
};

/// Enum <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_enum<T> >::type> {
  static octave_value to_octave(T const& v) {
    return octave_value(static_cast<int>(v));
  }
  static T from_octave(octave_value const& v) {
    return static_cast<T>(v.int_value());
  }
};

/// String <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_same<T, std::string> >::type > {
  static octave_value to_octave(T const& v) {
    return octave_value(v);
  }
  static T from_octave(octave_value const& v) {
    return v.string_value();
  }
};

/// Number <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_arithmetic<T> >::type> {
  static octave_value to_octave(T const& v) {
    return double(v);
  }
  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_scalar_type(), BadCastErr() << "Not a scalar type");
    return T(v.double_value());
  }
};

/// vw::Vector <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::math::VectorBase<T>, T> >::type> {
  static octave_value to_octave(T const& v) {
    T vw_vect(v.impl());
    ColumnVector oct_vect(vw_vect.size());

    for (unsigned i = 0; i < vw_vect.size(); i++) {
      oct_vect(i) = vw_vect[i];
    }

    return oct_vect;
  }
  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_matrix_type(), BadCastErr() << "Not a matrix type");

    T vw_vect;
    ColumnVector oct_vect = v.column_vector_value();

    if (vw_vect.size() == 0) {
      vw_vect = T(oct_vect.length());
    }

    VW_ASSERT(vw_vect.size() == unsigned(oct_vect.length()), BadCastErr() << "Bad vector size");

    for (int i = 0; i < oct_vect.length(); i++) {
      vw_vect[i] = oct_vect(i);
    }

    return vw_vect;
  }
};

/// vw::Quat <-> octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::math::QuaternionBase<T>, T> >::type> {
  static octave_value to_octave(T const& v) {
    T vw_quat(v.impl());
    ColumnVector oct_vect(4);

    for (unsigned i = 0; i < 4; i++) {
      oct_vect(i) = vw_quat[i];
    }

    return oct_vect;
  }
  static T from_octave(octave_value const& v) {
    return T(ConversionHelper<vw::Vector<double, 4> >::from_octave(v));
  }
};

/// vw::Matrix <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::math::MatrixBase<T>, T> >::type> {
  static octave_value to_octave(T const& v) {
    T vw_mat(v.impl());
    ::Matrix oct_mat(vw_mat.rows(), vw_mat.cols());
    
    for (unsigned col = 0; col < vw_mat.cols(); col++) {
      for (unsigned row = 0; row < vw_mat.rows(); row++) {
        oct_mat(row, col) = vw_mat(row, col);
      }
    }

    return oct_mat;
  }
  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_matrix_type(), BadCastErr() << "Not a matrix type");

    T vw_mat;
    ::Matrix oct_mat = v.matrix_value();

    if (vw_mat.rows() == 0 || vw_mat.cols() == 0) {
      vw_mat = T(oct_mat.rows(), oct_mat.cols());
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
};

/// vw::imageview <-> octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::ImageViewBase<T>, T> >::type> {
  static octave_value to_octave(T const& v) {
    typedef vw::ImageView<vw::PixelMask<double> > RasterT;

    // Rasterize image before copying to octave
    RasterT rast = v.impl();

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
  static T from_octave(octave_value const& v) {
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
};

/// Protobuffers <-> Octave

// Everything except Enum and Message
#define OCTAVE_CONV_protolist \
  ((CPPTYPE_INT32, Int32, int32)) \
  ((CPPTYPE_INT64, Int64, int64)) \
  ((CPPTYPE_UINT32, UInt32, uint32)) \
  ((CPPTYPE_UINT64, UInt64, uint64)) \
  ((CPPTYPE_DOUBLE, Double, double)) \
  ((CPPTYPE_FLOAT, Float, float)) \
  ((CPPTYPE_BOOL, Bool, bool)) \
  ((CPPTYPE_STRING, String, std::string)) 

#define OCTAVE_CONV_proto_to_octave(FTYPE, GETTER, TYPE) \
  case FieldDescriptor::FTYPE: \
    result.setfield(field_name, ConversionHelper<TYPE>::to_octave(reflection->Get##GETTER(v, field))); \
    break;

#define OCTAVE_CONV_proto_from_octave(FTYPE, SETTER, TYPE) \
  case FieldDescriptor::FTYPE: \
    if (vmap.contains(field_name)) { \
      reflection->Set##SETTER(m, field, ConversionHelper<TYPE>::from_octave(vmap.getfield(field_name))); \
    } else { \
      VW_ASSERT(!field->is_required(), BadCastErr() << "Missing required field in protobuf"); \
    } \
    break;

#define OCTAVE_CONV_protolist_iterator(r, d, i) OCTAVE_CONV_proto_##d i

template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<google::protobuf::Message, T> >::type> {
  static octave_value to_octave(T const& v) {
    using namespace google::protobuf;

    octave_scalar_map result;

    const Descriptor *descriptor = v.GetDescriptor();
    const Reflection *reflection = v.GetReflection();

    for (int i = 0; i < descriptor->field_count(); i++) {
      const FieldDescriptor *field = descriptor->field(i);

      const std::string field_name(field->name());

      switch(field->cpp_type()) {
        BOOST_PP_SEQ_FOR_EACH(OCTAVE_CONV_protolist_iterator, to_octave, OCTAVE_CONV_protolist)
        OCTAVE_CONV_proto_to_octave(CPPTYPE_MESSAGE, Message, Message)
        default:
          vw::vw_throw(vw::NoImplErr() << "Not a supported field type to convert to octave");
      }
    }

    return result;
  }

  static void from_octave_helper(octave_value const& v, google::protobuf::Message *m) {
    using namespace google::protobuf;

    octave_scalar_map vmap(v.scalar_map_value());

    const Descriptor *descriptor = m->GetDescriptor();
    const Reflection *reflection = m->GetReflection();

    for (int i = 0; i < descriptor->field_count(); i++) {
      const FieldDescriptor *field = descriptor->field(i);

      const std::string field_name(field->name());

      switch(field->cpp_type()) {
        BOOST_PP_SEQ_FOR_EACH(OCTAVE_CONV_protolist_iterator, from_octave, OCTAVE_CONV_protolist)
        case FieldDescriptor::CPPTYPE_MESSAGE:
          if (vmap.contains(field_name)) {
            from_octave_helper(vmap.getfield(field_name), reflection->MutableMessage(m, field));
          } else {
            VW_ASSERT(!field->is_required(), BadCastErr() << "Missing required field in protobuf");
          }
          break;
        default:
          vw::vw_throw(vw::NoImplErr() << "Not a supported field type to convert from octave");
      }
    }
  }

  static T from_octave(octave_value const& v) {
    T result;
    from_octave_helper(v, &result);
    return result;
  } 
};

#undef OCTAVE_CONV_protolist
#undef OCTAVE_CONV_proto_to_octave
#undef OCTAVE_CONV_proto_from_octave
#undef OCTAVE_CONV_protolist_iterator



}} // namespace octave, mvp

#endif
