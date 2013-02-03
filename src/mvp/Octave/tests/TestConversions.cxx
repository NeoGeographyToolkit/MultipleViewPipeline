#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Octave/Conversions.h>

#include <boost/random/linear_congruential.hpp>
#include <octave/oct-rand.h>

#include <vw/Image/UtilityViews.h>

using namespace mvp::octave;
using namespace vw;
using namespace vw::test;
using namespace std;

TEST(octave_wrap, scalar) {
  EXPECT_TRUE((octave_wrap(5) == octave_value(5)).bool_value());
}

TEST(octave_as, scalar) {
  EXPECT_EQ(octave_as<int>(octave_value(5)), 5);
}

TEST(octave_as, scalar_throws) {
  EXPECT_THROW(octave_as<int>(octave_map()), BadCastErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(octave_wrap, vector) {
  vw::Vector3 vw_vect(10, 20, 30);
  ColumnVector oct_vect(octave_wrap(vw_vect).column_vector_value());

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(octave_as, vector) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector3 vw_vect(octave_as<vw::Vector3>(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(octave_as, vector0) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector<double, 0> vw_vect(octave_as<vw::Vector<double, 0> >(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(octave_as, vector_throws) {
  EXPECT_THROW(octave_as<vw::Vector2>(ColumnVector(3)), BadCastErr);
  EXPECT_THROW(octave_as<vw::Vector2>(octave_map()), BadCastErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(octave_wrap, quaternion) {
  vw::Quat vw_quat(10, 20, 30, 40);
  ColumnVector oct_quat(octave_wrap(vw_quat).column_vector_value());

  for (unsigned i = 0; i < 4; i++) {
    EXPECT_EQ(vw_quat[i], oct_quat(i));
  }
}

TEST(octave_as, quaternion) {
  ColumnVector oct_quat(4);
  oct_quat(0) = 10; oct_quat(1) = 20; oct_quat(2) = 30; oct_quat(3) = 40;

  vw::Quat vw_quat(octave_as<vw::Quat>(oct_quat));

  for (unsigned i = 0; i < 4; i++) {
    EXPECT_EQ(vw_quat[i], oct_quat(i));
  }
}

TEST(octave_as, quaternion_throws) {
  EXPECT_THROW(octave_as<vw::Quat>(ColumnVector(3)), BadCastErr);
  EXPECT_THROW(octave_as<vw::Quat>(octave_map()), BadCastErr);
}

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(octave_wrap, matrix) {
  vw::Matrix<double, 2, 3> vw_mat;
  vw_mat(0, 0) = 10; vw_mat(0, 1) = 20; vw_mat(0, 2) = 30;
  vw_mat(1, 0) = 40; vw_mat(1, 1) = 30; vw_mat(1, 2) = 40;

  ::Matrix oct_mat(octave_wrap(vw_mat).matrix_value());

  for (unsigned r = 0; r < vw_mat.rows(); r++) {
    for (unsigned c = 0; c < vw_mat.cols(); c++) {
      EXPECT_EQ(vw_mat(r, c), oct_mat(r, c));
    }
  }
}

TEST(octave_as, matrix) {
  ::Matrix oct_mat(2, 3);
  oct_mat(0, 0) = 10; oct_mat(0, 1) = 20; oct_mat(0, 2) = 30;
  oct_mat(1, 0) = 40; oct_mat(1, 1) = 30; oct_mat(1, 2) = 40;

  vw::Matrix<double, 2, 3> vw_mat(octave_as<vw::Matrix<double, 2, 3> >(oct_mat));

  for (unsigned r = 0; r < vw_mat.rows(); r++) {
    for (unsigned c = 0; c < vw_mat.cols(); c++) {
      EXPECT_EQ(vw_mat(r, c), oct_mat(r, c));
    }
  }
}

TEST(octave_as, matrix0) {
  ::Matrix oct_mat(2, 3);
  oct_mat(0, 0) = 10; oct_mat(0, 1) = 20; oct_mat(0, 2) = 30;
  oct_mat(1, 0) = 40; oct_mat(1, 1) = 30; oct_mat(1, 2) = 40;

  vw::Matrix<double> vw_mat(octave_as<vw::Matrix<double> >(oct_mat));

  for (unsigned r = 0; r < vw_mat.rows(); r++) {
    for (unsigned c = 0; c < vw_mat.cols(); c++) {
      EXPECT_EQ(vw_mat(r, c), oct_mat(r, c));
    }
  }
}

TEST(octave_as, matrix_throws) {
  EXPECT_THROW(octave_as<vw::Matrix3x3>(::Matrix(3, 2)), BadCastErr);
  EXPECT_THROW(octave_as<vw::Matrix3x3>(octave_map()), BadCastErr);
} 

///////////////////////////////////////////////////////////////////////////////////////////////

TEST(octave_wrap, imageview) {
  boost::rand48 gen(10);

  ImageView<float32> vw_img(uniform_noise_view(gen, 20, 10));
  
  ::Matrix oct_img(octave_wrap(vw_img).matrix_value());

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_EQ(vw_img(col, row), oct_img(row, col));
    }
  }
}

TEST(octave_wrap, imageview_mask) {
  ImageView<PixelMask<float32> > vw_img(20, 10);
  
  ::Matrix oct_img(octave_wrap(vw_img).matrix_value());
 
  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_TRUE(::xisnan(oct_img(row, col)));
    }
  }
}

TEST(octave_wrap, imageview_mask2) {
  boost::rand48 gen(10);

  ImageView<PixelMask<float32> > vw_img(uniform_noise_view(gen, 10, 10));
  vw_img(5, 5).invalidate();

  ::Matrix oct_img(octave_wrap(vw_img).matrix_value());

  ImageView<PixelMask<float32> > vw_img2(octave_as<ImageView<PixelMask<float32> > >(oct_img));

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

TEST(octave_as, imageview) {
  ::octave_rand::seed(10);

  ::Matrix oct_img(::octave_rand::matrix(20, 10));
  ImageView<float32> vw_img(octave_as<ImageView<float32> >(oct_img));

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_PIXEL_NEAR(vw_img(col, row), oct_img(row, col), 1e-6);
    }
  }
}

TEST(octave_as, imageview_mask) {
  ::octave_rand::seed(10);

  ::Matrix oct_img(::octave_rand::matrix(20, 10));
  oct_img(0, 0) = ::octave_NA;

  ImageView<PixelMask<float32> > vw_img = octave_as<ImageView<PixelMask<float32> > >(oct_img);

  EXPECT_FALSE(is_valid(vw_img(0, 0)));
  for (int col = 1; col < vw_img.cols(); col++) {
    for (int row = 1; row < vw_img.rows(); row++) {
      EXPECT_TRUE(is_valid(vw_img(col, row)));
    }
  }
}
