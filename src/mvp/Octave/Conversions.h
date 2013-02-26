/// \file oct-cast.h
///
/// octave_as function
///

#ifndef __MVP_OCTAVE_OCTCAST_H__
#define __MVP_OCTAVE_OCTCAST_H__

#include <mvp/Octave/oct-mvpclass.h>

#include <octave/lo-ieee.h> // octave_NA

#include <vw/Math/Vector.h>
#include <vw/Math/Matrix.h>
#include <vw/Math/Quaternion.h>
#include <vw/Image/ImageView.h>
#include <vw/Image/PixelMask.h>


namespace mvp {
namespace octave {

VW_DEFINE_EXCEPTION(BadCastErr, vw::Exception);

template <class T, class Enable=void>
struct octave_wrap_helper {
  static octave_value wrap(T const& v) {
    boost::shared_ptr<octave_mvpclass_base> ptr(new octave_mvpclass_wrap<T>(v));
    return octave_value(new octave_mvpclass_ref(ptr));
  }
  static T as(octave_value const& v) {
    octave_mvpclass_ref *ref = dynamic_cast<octave_mvpclass_ref*>(v.internal_rep());
    VW_ASSERT(ref, BadCastErr() << "Not an mvpclass");
    boost::shared_ptr<octave_mvpclass_wrap<T> > wrap = boost::dynamic_pointer_cast<octave_mvpclass_wrap<T> >(ref->ptr());
    VW_ASSERT(wrap, BadCastErr() << "No OctaveWrapper for type");
    return wrap->impl();
  }
};

template <class T>
octave_value octave_wrap(T const& v) {
  return octave_wrap_helper<T>::wrap(v);
}

template <class T>
T octave_as(octave_value const& v) {
  return octave_wrap_helper<T>::as(v);
}

/// Specializations...

/// Enum <-> Octave
template <class T>
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_enum<T> >::type> {
  static octave_value wrap(T const& v) {
    return octave_value(static_cast<int>(v));
  }
  static T as(octave_value const& v) {
    return static_cast<T>(v.int_value());
  }
};

/// String <-> Octave
template <class T>
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_same<T, std::string> >::type > {
  static octave_value wrap(T const& v) {
    return octave_value(v);
  }
  static T as(octave_value const& v) {
    return v.string_value();
  }
};

/// Algorithm <-> Octave
template <class T>
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_class<typename T::algorithm_type> >::type > {
  static octave_value wrap(T const& v) {
    boost::shared_ptr<octave_mvpclass_base> ptr(new octave_mvpclass_wrap<T>(v));
    return octave_value(new octave_mvpclass_ref(ptr));
  }
  static T as(octave_value const& v) {
    octave_mvpclass_ref *ref = dynamic_cast<octave_mvpclass_ref*>(v.internal_rep());
    VW_ASSERT(ref, BadCastErr() << "Not an mvpclass");
    boost::shared_ptr<octave_mvpclass_wrap<T> > wrap = boost::dynamic_pointer_cast<octave_mvpclass_wrap<T> >(ref->ptr());
    if (wrap) {
      return wrap->impl();
    } else {
      // TODO: return an OctaveWrap of the class
      return T();
    }
  }
};

/// Number <-> Octave
template <class T>
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_arithmetic<T> >::type> {
  static octave_value wrap(T const& v) {
    return double(v);
  }
  static T as(octave_value const& v) {
    VW_ASSERT(v.is_scalar_type(), BadCastErr() << "Not a scalar type");
    return T(v.double_value());
  }
};

/// vw::Vector <-> Octave
template <class T>
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_base_of<vw::math::VectorBase<T>, T> >::type> {
  static octave_value wrap(T const& v) {
    T vw_vect(v.impl());
    ColumnVector oct_vect(vw_vect.size());

    for (unsigned i = 0; i < vw_vect.size(); i++) {
      oct_vect(i) = vw_vect[i];
    }

    return oct_vect;
  }
  static T as(octave_value const& v) {
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
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_base_of<vw::math::QuaternionBase<T>, T> >::type> {
  static octave_value wrap(T const& v) {
    T vw_quat(v.impl());
    ColumnVector oct_vect(4);

    for (unsigned i = 0; i < 4; i++) {
      oct_vect(i) = vw_quat[i];
    }

    return oct_vect;
  }
  static T as(octave_value const& v) {
    return T(octave_as<vw::Vector<double, 4> >(v));
  }
};

/// vw::Matrix <-> Octave
template <class T>
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_base_of<vw::math::MatrixBase<T>, T> >::type> {
  static octave_value wrap(T const& v) {
    T vw_mat(v.impl());
    ::Matrix oct_mat(vw_mat.rows(), vw_mat.cols());
    
    for (unsigned col = 0; col < vw_mat.cols(); col++) {
      for (unsigned row = 0; row < vw_mat.rows(); row++) {
        oct_mat(row, col) = vw_mat(row, col);
      }
    }

    return oct_mat;
  }
  static T as(octave_value const& v) {
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
struct octave_wrap_helper<T, typename boost::enable_if<boost::is_base_of<vw::ImageViewBase<T>, T> >::type> {
  static octave_value wrap(T const& v) {
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
  static T as(octave_value const& v) {
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

}} // namespace octave, mvp

#endif
