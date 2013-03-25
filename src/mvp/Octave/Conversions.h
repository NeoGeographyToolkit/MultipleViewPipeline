/// \file oct-cast.h
///
/// Conversions to/from octave
///

#ifndef __MVP_OCTAVE_OCTCAST_H__
#define __MVP_OCTAVE_OCTCAST_H__

#include <mvp/Octave/oct-mvpclass.h>
#include <mvp/Octave/oct-typetraits.h>

#include <google/protobuf/message.h>
#include <google/protobuf/descriptor.h>

#include <vw/Math/Vector.h>
#include <vw/Math/Matrix.h>
#include <vw/Math/Quaternion.h>
#include <vw/Image/ImageView.h>
#include <vw/Image/Manipulation.h>
#include <vw/Image/PixelMask.h>

#include <boost/preprocessor/seq/for_each.hpp>

namespace mvp {
namespace octave {

VW_DEFINE_EXCEPTION(BadCastErr, vw::Exception);

template <class T, class Enable=void, class Disable=void>
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

/// allow void to_octave for OctaveWrappers
template <>
struct ConversionHelper<void> {
  static void from_octave(octave_value const& v) {}
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

/// Floating point <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_floating_point<T> >::type> {
  static octave_value to_octave(T const& v) {
    return v;
  }

  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_scalar_type(), BadCastErr() << "Not a scalar type");
    return octave_value_extract<T>(v);
  }
};

/// Integral <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_integral<T> >::type,
                           typename boost::disable_if<boost::is_same<T, bool> >::type> {
  static octave_value to_octave(T const& v) {
    return octave_int<T>(v);
  }

  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_scalar_type(), BadCastErr() << "Not a scalar type");
    return octave_value_extract<octave_int<T> >(v);
  }
};

/// bool <-> Octave
template <>
struct ConversionHelper<bool> {
  static octave_value to_octave(bool const& v) {
    return v;
  }

  static bool from_octave(octave_value const& v) {
    VW_ASSERT(v.is_scalar_type(), BadCastErr() << "Not a scalar type");
    return octave_value_extract<bool>(v);
  }
};

/// Enum <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_enum<T> >::type> {
  static octave_value to_octave(T const& v) {
    return octave_value(ConversionHelper<int>::to_octave(static_cast<int>(v)));
  }

  static T from_octave(octave_value const& v) {
    return static_cast<T>(ConversionHelper<int>::from_octave(v));
  }
};

/// String <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_same<T, std::string> >::type > {
  static octave_value to_octave(T const& v) {
    return octave_value(v);
  }

  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_string(), BadCastErr() << "Not a string type");
    return v.string_value();
  }
};


/// vw::Vector <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::math::VectorBase<T>, T> >::type> {
  typedef typename T::value_type VT;
  typedef typename octave_array_type<VT>::type AT;
  typedef typename octave_array_type<VT>::value_type AVT;

  static octave_value to_octave(T const& vw_vect) {
    AT oct_vect(dim_vector(vw_vect.size(), 1));

    std::copy(vw_vect.begin(), vw_vect.end(), const_cast<AVT*>(oct_vect.data()));

    return oct_vect;
  }

  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_matrix_type(), BadCastErr() << "Not a matrix type");

    T vw_vect;
    AT oct_vect = octave_value_extract<AT>(v);

    if (vw_vect.size() == 0) {
      vw_vect = T(oct_vect.length());
    }

    VW_ASSERT(vw_vect.size() == unsigned(oct_vect.length()), BadCastErr() << "Bad vector size");

    std::copy(oct_vect.data(), oct_vect.data() + oct_vect.numel(), vw_vect.begin());

    return vw_vect;
  }
};

/// vw::Quat <-> octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::math::QuaternionBase<T>, T> >::type> {
  typedef typename T::value_type VT;
  typedef typename octave_array_type<VT>::type AT;

  static octave_value to_octave(T const& vw_quat) {
    AT oct_vect(dim_vector(4, 1));

    for (unsigned i = 0; i < 4; i++) {
      oct_vect(i) = vw_quat[i];
    }

    return oct_vect;
  }

  static T from_octave(octave_value const& v) {
    return T(ConversionHelper<vw::Vector<VT, 4> >::from_octave(v));
  }
};

/// vw::Matrix <-> Octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::math::MatrixBase<T>, T> >::type> {
  typedef typename T::value_type VT;
  typedef typename octave_array_type<VT>::type AT;
  typedef typename octave_array_type<VT>::value_type AVT;

  static octave_value to_octave(T const& v) {
    T vw_mat(v);
    AT oct_mat(dim_vector(vw_mat.rows(), vw_mat.cols()));

    vw::math::MatrixTranspose<T> vw_mat_trans(vw_mat);
    std::copy(vw_mat_trans.begin(), vw_mat_trans.end(), const_cast<AVT*>(oct_mat.data()));
    
    return oct_mat;
  }
  
  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_matrix_type(), BadCastErr() << "Not a matrix type");

    T vw_mat;
    AT oct_mat = octave_value_extract<AT>(v);

    if (vw_mat.rows() == 0 || vw_mat.cols() == 0) {
      vw_mat = T(oct_mat.rows(), oct_mat.cols());
    }

    VW_ASSERT(vw_mat.cols() == unsigned(oct_mat.cols()), BadCastErr() << "Bad matrix size");
    VW_ASSERT(vw_mat.rows() == unsigned(oct_mat.rows()), BadCastErr() << "Bad matrix size");

    vw::math::MatrixTranspose<T> vw_mat_trans(vw_mat);
    std::copy(oct_mat.data(), oct_mat.data() + oct_mat.numel(), vw_mat_trans.begin());

    return vw_mat;
  }
};

/// vw::imageview <-> octave
template <class T>
struct ConversionHelper<T, typename boost::enable_if<boost::is_base_of<vw::ImageViewBase<T>, T> >::type> {
  typedef typename vw::CompoundChannelType<typename T::pixel_type>::type VT;
  typedef typename octave_array_type<VT>::type AT;
  typedef typename octave_array_type<VT>::value_type AVT;
  const static int num_channels = vw::CompoundNumChannels<typename T::pixel_type>::value;

  static octave_value to_octave(T const& v) {
    VW_ASSERT(num_channels == 1 || v.planes() == 1, 
      BadCastErr() << "Cannot convert multi-channel-multi-plane image");

    vw::ImageView<VT> vw_img;

    // Rasterize image before copying to octave
    if (num_channels == 1) {
      vw_img = vw::pixel_cast<VT>(select_channel(v, 0));
    } else {
      vw_img = vw::pixel_cast<VT>(vw::channels_to_planes(v));
    }

    AT oct_img(dim_vector(vw_img.rows(), vw_img.cols(), vw_img.planes()));

    vw::TransposeView<vw::ImageView<VT> > vw_img_trans(vw_img);
    std::copy(vw_img_trans.begin(), vw_img_trans.end(), const_cast<AVT*>(oct_img.data()));

    return oct_img;
  }

  static T from_octave(octave_value const& v) {
    VW_ASSERT(v.is_matrix_type(), BadCastErr() << "Not a matrix type");

    AT oct_img = octave_value_extract<AT>(v);
    
    vw::ImageView<VT> vw_img(oct_img.cols(), oct_img.rows(),
                             oct_img.dims().length() < 3 ? 1 : oct_img.dim3());

    vw::TransposeView<vw::ImageView<VT> > vw_img_trans(vw_img);
    std::copy(oct_img.data(), oct_img.data() + oct_img.numel(), vw_img_trans.begin());

    if (num_channels == 1) {
      return vw::pixel_cast<typename T::pixel_type>(vw_img);
    } else {
      return vw::planes_to_channels<typename T::pixel_type>(vw_img);
    }
  }
};

/// std::vector <-> Octave
template <class T>
struct ConversionHelper<std::vector<T> > {
  static octave_value to_octave(std::vector<T> const& v) {
    octave_value_list result;

    for (unsigned i = 0; i < v.size(); i++) {
      result.append(ConversionHelper<T>::to_octave(v[i]));
    }

    return octave_value(Cell(result));
  }

  static std::vector<T> from_octave(octave_value const& v) {
    VW_ASSERT(v.is_cell(), BadCastErr() << "Not a cell array");
    Cell oct_cell = v.cell_value();

    std::vector<T> result(oct_cell.numel());

    for (int i = 0; i < oct_cell.numel(); i++) {
      result[i] = ConversionHelper<T>::from_octave(oct_cell(i));
    }

    return result;
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
    VW_ASSERT(v.is_map(), BadCastErr() << "Not a map type");
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
