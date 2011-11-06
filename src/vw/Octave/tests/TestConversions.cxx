#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <vw/Octave/Conversions.h>
#include <vw/Image/UtilityViews.h>

#include <boost/random/linear_congruential.hpp>
#include <octave/oct-rand.h>

using namespace vw;
using namespace vw::camera;
using namespace vw::cartography;
using namespace vw::octave;
using namespace vw::test;

TEST(Conversions, vector_to_octave) {
  vw::Vector3 vw_vect(10, 20, 30);
  ::ColumnVector oct_vect(vector_to_octave(vw_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, octave_to_vector) {
  ::ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector3 vw_vect(octave_to_vector(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, pinhole_to_octave) {
  PinholeModel vw_cam(SrcName("AS15-M-0073.lev1.pinhole"));
  ::Matrix oct_cam(pinhole_to_octave(vw_cam));

  {
    vw::Vector3 pt(0, 0, 0);
    Vector2 vw_result = vw_cam.point_to_pixel(pt);
    Vector2 oct_result = dehom(octave_to_vector(oct_cam * vector_to_octave(hom(pt)))) - Vector2(1, 1);
    EXPECT_VECTOR_NEAR(vw_result, oct_result, 1e-6);
  }

  {
    vw::Vector3 pt(1003, 29, -8200);
    Vector2 vw_result = vw_cam.point_to_pixel(pt);
    Vector2 oct_result = dehom(octave_to_vector(oct_cam * vector_to_octave(hom(pt)))) - Vector2(1, 1);
    EXPECT_VECTOR_NEAR(vw_result, oct_result, 1e-6);
  }
}

TEST(Conversions, imageview_to_octave) {
  boost::rand48 gen(10);

  ImageView<float32> vw_img(uniform_noise_view(gen, 10, 10));
  ::Matrix oct_img(imageview_to_octave(vw_img));

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_EQ(vw_img(col, row), oct_img(row, col));
    }
  }
}

TEST(Conversions, octave_to_imageview) {
  ::octave_rand::seed(10);

  ::Matrix oct_img(::octave_rand::matrix(10, 10));
  ImageView<float32> vw_img(octave_to_imageview(oct_img));

  for (int col = 0; col < vw_img.cols(); col++) {
    for (int row = 0; row < vw_img.rows(); row++) {
      EXPECT_PIXEL_NEAR(vw_img(col, row), oct_img(row, col), 1e-6);
    }
  }
}

TEST(Conversions, imageview_to_octave_masked) {
  boost::rand48 gen(10);

  ImageView<PixelMask<float32> > vw_img(uniform_noise_view(gen, 10, 10));
  vw_img(5, 5).invalidate();

  ::Matrix oct_img(imageview_to_octave(vw_img));

  ImageView<PixelMask<float32> > vw_img2(octave_to_imageview(oct_img));

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

TEST(Conversions, georef_to_octave) {
  double res = 256;

  Matrix3x3 transform;
  transform(0,0) = 360.0 / res;
  transform(0,2) = -180.0;
  transform(1,1) = -360.0 / res;
  transform(1,2) = 180.0;
  transform(2,2) = 1;

  Datum datum;

  GeoReference vw_geo(datum, transform);
  ::octave_scalar_map oct_geo(georef_to_octave(vw_geo));

  { 
    Vector2 px(0, 0);
    Vector2 vw_result = vw_geo.pixel_to_lonlat(px) * M_PI / 180.0; // Convert to radians
    Vector2 oct_result = dehom(octave_to_vector(oct_geo.getfield("transform").matrix_value() * vector_to_octave(hom(px + Vector2(1, 1)))));
    EXPECT_VECTOR_NEAR(vw_result, oct_result, 1e-6);
  }

  { 
    Vector2 px(100, -30);
    Vector2 vw_result = vw_geo.pixel_to_lonlat(px) * M_PI / 180.0; // Convert to radians
    Vector2 oct_result = dehom(octave_to_vector(oct_geo.getfield("transform").matrix_value() * vector_to_octave(hom(px + Vector2(1, 1)))));
    EXPECT_VECTOR_NEAR(vw_result, oct_result, 1e-6);
  }

  EXPECT_EQ(oct_geo.getfield("datum").scalar_map_value().getfield("semi_major_axis").double_value(), datum.semi_major_axis());
  EXPECT_EQ(oct_geo.getfield("datum").scalar_map_value().getfield("semi_minor_axis").double_value(), datum.semi_minor_axis());
}
