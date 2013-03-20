#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Octave/Conversions.h>

#include <boost/random/linear_congruential.hpp>
#include <octave/oct-rand.h>

#include <vw/Image/UtilityViews.h>

#include <mvp/Octave/tests/DummyProto.pb.h>

using namespace mvp::octave;
using namespace vw;
using namespace vw::test;
using namespace std;

TEST(to_octave, scalar) {
  EXPECT_TRUE((to_octave(5) == octave_value(5)).bool_value());
}

TEST(from_octave, scalar) {
  EXPECT_EQ(from_octave<int>(octave_value(5)), 5);
}

TEST(from_octave, scalar_throws) {
  EXPECT_THROW(from_octave<int>(octave_map()), BadCastErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(to_octave, vector) {
  vw::Vector3 vw_vect(10, 20, 30);
  ColumnVector oct_vect(to_octave(vw_vect).column_vector_value());

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(from_octave, vector) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector3 vw_vect(from_octave<vw::Vector3>(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(from_octave, vector0) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector<double, 0> vw_vect(from_octave<vw::Vector<double, 0> >(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(from_octave, vector_throws) {
  EXPECT_THROW(from_octave<vw::Vector2>(ColumnVector(3)), BadCastErr);
  EXPECT_THROW(from_octave<vw::Vector2>(octave_map()), BadCastErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(to_octave, quaternion) {
  vw::Quat vw_quat(10, 20, 30, 40);
  ColumnVector oct_quat(to_octave(vw_quat).column_vector_value());

  for (unsigned i = 0; i < 4; i++) {
    EXPECT_EQ(vw_quat[i], oct_quat(i));
  }
}

TEST(from_octave, quaternion) {
  ColumnVector oct_quat(4);
  oct_quat(0) = 10; oct_quat(1) = 20; oct_quat(2) = 30; oct_quat(3) = 40;

  vw::Quat vw_quat(from_octave<vw::Quat>(oct_quat));

  for (unsigned i = 0; i < 4; i++) {
    EXPECT_EQ(vw_quat[i], oct_quat(i));
  }
}

TEST(from_octave, quaternion_throws) {
  EXPECT_THROW(from_octave<vw::Quat>(ColumnVector(3)), BadCastErr);
  EXPECT_THROW(from_octave<vw::Quat>(octave_map()), BadCastErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(to_octave, matrix) {
  vw::Matrix<double, 2, 3> vw_mat;
  vw_mat(0, 0) = 10; vw_mat(0, 1) = 20; vw_mat(0, 2) = 30;
  vw_mat(1, 0) = 40; vw_mat(1, 1) = 30; vw_mat(1, 2) = 40;

  ::Matrix oct_mat(to_octave(vw_mat).matrix_value());

  for (unsigned r = 0; r < vw_mat.rows(); r++) {
    for (unsigned c = 0; c < vw_mat.cols(); c++) {
      EXPECT_EQ(vw_mat(r, c), oct_mat(r, c));
    }
  }
}

TEST(from_octave, matrix) {
  ::Matrix oct_mat(2, 3);
  oct_mat(0, 0) = 10; oct_mat(0, 1) = 20; oct_mat(0, 2) = 30;
  oct_mat(1, 0) = 40; oct_mat(1, 1) = 30; oct_mat(1, 2) = 40;

  vw::Matrix<double, 2, 3> vw_mat(from_octave<vw::Matrix<double, 2, 3> >(oct_mat));

  for (unsigned r = 0; r < vw_mat.rows(); r++) {
    for (unsigned c = 0; c < vw_mat.cols(); c++) {
      EXPECT_EQ(vw_mat(r, c), oct_mat(r, c));
    }
  }
}

TEST(from_octave, matrix0) {
  ::Matrix oct_mat(2, 3);
  oct_mat(0, 0) = 10; oct_mat(0, 1) = 20; oct_mat(0, 2) = 30;
  oct_mat(1, 0) = 40; oct_mat(1, 1) = 30; oct_mat(1, 2) = 40;

  vw::Matrix<double> vw_mat(from_octave<vw::Matrix<double> >(oct_mat));

  for (unsigned r = 0; r < vw_mat.rows(); r++) {
    for (unsigned c = 0; c < vw_mat.cols(); c++) {
      EXPECT_EQ(vw_mat(r, c), oct_mat(r, c));
    }
  }
}

TEST(from_octave, matrix_throws) {
  EXPECT_THROW(from_octave<vw::Matrix3x3>(::Matrix(3, 2)), BadCastErr);
  EXPECT_THROW(from_octave<vw::Matrix3x3>(octave_map()), BadCastErr);
} 

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(to_octave, imageview) {
  boost::rand48 gen(10);

  ImageView<float32> vw_img(uniform_noise_view(gen, 20, 10));
  
  ::Matrix oct_img(to_octave(vw_img).matrix_value());

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_EQ(vw_img(col, row), oct_img(row, col));
    }
  }
}

TEST(to_octave, imageview_mask) {
  ImageView<PixelMask<float32> > vw_img(20, 10);
  
  ::Matrix oct_img(to_octave(vw_img).matrix_value());
 
  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_TRUE(::xisnan(oct_img(row, col)));
    }
  }
}

TEST(to_octave, imageview_mask2) {
  boost::rand48 gen(10);

  ImageView<PixelMask<float32> > vw_img(uniform_noise_view(gen, 10, 10));
  vw_img(5, 5).invalidate();

  ::Matrix oct_img(to_octave(vw_img).matrix_value());

  ImageView<PixelMask<float32> > vw_img2(from_octave<ImageView<PixelMask<float32> > >(oct_img));

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      if (is_valid(vw_img(col, row))) {
        EXPECT_PIXEL_NEAR(vw_img(col, row), vw_img2(col, row), 1e-6);
      } else {
        EXPECT_FALSE(is_valid(vw_img2(col, row)));
      }
    }
  }
}

TEST(from_octave, imageview) {
  ::octave_rand::seed(10);

  ::Matrix oct_img(::octave_rand::matrix(20, 10));
  ImageView<float32> vw_img(from_octave<ImageView<float32> >(oct_img));

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_PIXEL_NEAR(vw_img(col, row), oct_img(row, col), 1e-6);
    }
  }
}

TEST(from_octave, imageview_mask) {
  ::octave_rand::seed(10);

  ::Matrix oct_img(::octave_rand::matrix(20, 10));
  oct_img(0, 0) = ::octave_NA;

  ImageView<PixelMask<float32> > vw_img = from_octave<ImageView<PixelMask<float32> > >(oct_img);

  EXPECT_FALSE(is_valid(vw_img(0, 0)));
  for (int col = 1; col < vw_img.cols(); col++) {
    for (int row = 1; row < vw_img.rows(); row++) {
      EXPECT_TRUE(is_valid(vw_img(col, row)));
    }
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(to_octave, protobuffer) {
  DummyProto message;
  message.set_double_field(0.1);
  message.set_float_field(0.2);
  message.set_int32_field(3);
  message.set_bool_field(true);
  message.set_string_field("4");
  message.mutable_message_field()->set_field(5);

  octave_scalar_map oct_map(to_octave(message).scalar_map_value());

  EXPECT_EQ(oct_map.getfield("double_field").double_value(), message.double_field());
  EXPECT_EQ(oct_map.getfield("float_field").float_value(), message.float_field());
  EXPECT_EQ(oct_map.getfield("int32_field").int_value(), message.int32_field());
  EXPECT_EQ(oct_map.getfield("bool_field").bool_value(), message.bool_field());
  EXPECT_EQ(oct_map.getfield("string_field").string_value(), message.string_field());
  EXPECT_EQ(oct_map.getfield("empty_field").int_value(), message.empty_field());
  EXPECT_EQ(oct_map.getfield("message_field").scalar_map_value().getfield("field").int_value(),
            message.message_field().field());
}

TEST(from_octave, protobuffer) {
  octave_scalar_map oct_map;
  oct_map.setfield("double_field", 0.1);
  oct_map.setfield("float_field", 0.2);
  oct_map.setfield("int32_field", 3);
  oct_map.setfield("bool_field", true);
  oct_map.setfield("string_field", "4");

  EXPECT_THROW(from_octave<DummyProto>(oct_map), BadCastErr); // Missing required field

  octave_scalar_map inner_oct_map;
  inner_oct_map.setfield("field", 10);
  oct_map.setfield("message_field", inner_oct_map);

  DummyProto message = from_octave<DummyProto>(oct_map);

  EXPECT_EQ(oct_map.getfield("double_field").double_value(), message.double_field());
  EXPECT_EQ(oct_map.getfield("float_field").float_value(), message.float_field());
  EXPECT_EQ(oct_map.getfield("int32_field").int_value(), message.int32_field());
  EXPECT_EQ(oct_map.getfield("bool_field").bool_value(), message.bool_field());
  EXPECT_EQ(oct_map.getfield("string_field").string_value(), message.string_field());
  EXPECT_EQ(oct_map.getfield("message_field").scalar_map_value().getfield("field").int_value(),
            message.message_field().field());
}
