#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Octave/oct-cast.h>

#include <boost/random/linear_congruential.hpp>
#include <octave/oct-rand.h>

#include <vw/Image/UtilityViews.h>

using namespace mvp::octave;
using namespace vw;
using namespace vw::test;
using namespace std;

TEST(Conversions, scalar_to_octave) {
  EXPECT_TRUE((octave_cast<octave_value>(5) == octave_value(5)).bool_value());
}

TEST(Conversions, octave_to_scalar) {
  EXPECT_EQ(octave_cast<int>(octave_value(5)), 5);
}

TEST(Conversions, octave_to_scalar_throws) {
  EXPECT_THROW(octave_cast<int>(octave_map()), BadCastErr);
}

TEST(Conversions, vector_to_octave) {
  vw::Vector3 vw_vect(10, 20, 30);
  ColumnVector oct_vect(octave_cast<ColumnVector>(vw_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, octave_to_vector) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector3 vw_vect(octave_cast<vw::Vector3>(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, octave_to_vector0) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector<double, 0> vw_vect(octave_cast<vw::Vector<double, 0> >(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, octave_to_vector_throws) {
  EXPECT_THROW(octave_cast<vw::Vector2>(ColumnVector(3)), BadCastErr);
  EXPECT_THROW(octave_cast<vw::Vector2>(octave_map()), BadCastErr);
}

TEST(Conversions, quat_to_octave) {
  vw::Quat vw_quat(10, 20, 30, 40);
  ColumnVector oct_quat(octave_cast<ColumnVector>(vw_quat));

  for (unsigned i = 0; i < 4; i++) {
    EXPECT_EQ(vw_quat[i], oct_quat(i));
  }
}

TEST(Conversions, octave_to_quat) {
  ColumnVector oct_quat(4);
  oct_quat(0) = 10; oct_quat(1) = 20; oct_quat(2) = 30; oct_quat(3) = 40;

  vw::Quat vw_quat(octave_cast<vw::Quat>(oct_quat));

  for (unsigned i = 0; i < 4; i++) {
    EXPECT_EQ(vw_quat[i], oct_quat(i));
  }
}

TEST(Conversions, octave_to_quat_throws) {
  EXPECT_THROW(octave_cast<vw::Quat>(ColumnVector(3)), BadCastErr);
  EXPECT_THROW(octave_cast<vw::Quat>(octave_map()), BadCastErr);
}

TEST(Conversions, imageview_to_octave) {
  boost::rand48 gen(10);

  ImageView<float32> vw_img(uniform_noise_view(gen, 20, 10));
  
  ::Matrix oct_img(octave_cast< ::Matrix>(vw_img));

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_EQ(vw_img(col, row), oct_img(row, col));
    }
  }
}

/*
TEST(Conversions, imageview_to_octave_mask) {
  ImageView<PixelMask<float32> > vw_img(20, 10);
  
  ::Matrix oct_img(octave_cast< ::Matrix>(vw_img));
 
  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_TRUE(::xisnan(oct_img(row, col)));
    }
  }
}

TEST(Conversions, imageview_to_octave_mask2) {
  boost::rand48 gen(10);

  ImageView<PixelMask<float32> > vw_img(uniform_noise_view(gen, 10, 10));
  vw_img(5, 5).invalidate();

  ::Matrix oct_img(octave_cast< ::Matrix>(vw_img));

  ImageView<PixelMask<float32> > vw_img2(octave_cast<ImageView<PixelMask<float32> > >(oct_img));

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

TEST(Conversions, octave_to_imageview) {
  ::octave_rand::seed(10);

  ::Matrix oct_img(::octave_rand::matrix(20, 10));
  ImageView<float32> vw_img(octave_cast<ImageView<float32> >(oct_img));

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_PIXEL_NEAR(vw_img(col, row), oct_img(row, col), 1e-6);
    }
  }
}

TEST(Conversions, octave_to_imageview_mask) {
  ::octave_rand::seed(10);

  ::Matrix oct_img(::octave_rand::matrix(20, 10));
  oct_img(0, 0) = ::octave_NA;

  ImageView<PixelMask<float32> > vw_img = octave_cast<ImageView<PixelMask<float32> > >(oct_img);

  EXPECT_FALSE(is_valid(vw_img(0, 0)));
  for (int col = 1; col < vw_img.cols(); col++) {
    for (int row = 1; row < vw_img.rows(); row++) {
      EXPECT_TRUE(is_valid(vw_img(col, row)));
    }
  }
}
*/
