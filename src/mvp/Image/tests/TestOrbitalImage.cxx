#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Image/OrbitalImage.h>

#include <vw/Camera/PinholeModel.h>
#include <vw/Image/UtilityViews.h>
#include <vw/Image/Filter.h>
#include <vw/Image/MaskViews.h>
#include <vw/Image/Statistics.h>

#include <boost/random/linear_congruential.hpp>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace mvp::image;

template <class Image1T, class Image2T>
bool is_image_valid_eq(ImageViewBase<Image1T> const& img1, ImageViewBase<Image2T> const& img2, double tol) {
  double diff = sum_of_channel_values(abs(img1.impl() - img2.impl()));
  return !is_transparent(img1) && !is_transparent(img2) && diff < tol; 
}

OrbitalImage synth_orbit(Vector3 const& camera_center, Vector3 const& axis_angle, 
                         Vector2 const& focus, Vector2i size)
{
  boost::rand48 gen;
  PinholeModel camera(camera_center, axis_angle_to_matrix(axis_angle), 
                      focus.x(), focus.y(), size.x() / 2.0, size.y() / 2.0);

  ImageView<OrbitalImagePixel> img = pixel_cast<OrbitalImagePixel>(gaussian_filter(uniform_noise_view(gen, size.x(), size.y()), 1.5));
  OrbitalImage orbital_image(img, camera);
  return orbital_image; 
}

TEST(OrbitalImage, back_project) {
  Vector2i size(64, 32);

  OrbitalImage orbital_image;
  ImageView<OrbitalImagePixel> patch;

  // Camera at z = 1, looking back at origin. Plane at z = 0
  orbital_image = synth_orbit(Vector3(0, 0, 1), Vector3(M_PI, 0, 0), Vector2(1, 1), size);
  patch = orbital_image.back_project(Vector3(0, 0, 0), Quat(1, 0, 0, 0), Vector2(1, 1), size);
  EXPECT_TRUE(is_image_valid_eq(orbital_image.image(), patch, 1e-6));

  // Camera at z = 1, looking back at origin. Plane at z = -1
  // Patch is 2f from plane, so to get the original image we make patch with scale 2m/px
  orbital_image = synth_orbit(Vector3(0, 0, 1), Vector3(M_PI, 0, 0), Vector2(1, 1), size);
  patch = orbital_image.back_project(Vector3(0, 0, -1), Quat(1, 0, 0, 0), Vector2(2, 2), size);
  EXPECT_TRUE(is_image_valid_eq(orbital_image.image(), patch, 1e-6));

  // Camera at z = -1, looking toward origin. Plane at z = 0, rotated 180deg about x axis (vertical flip)
  orbital_image = synth_orbit(Vector3(0, 0, -1), Vector3(0, 0, 0), Vector2(1, 1), size);
  patch = orbital_image.back_project(Vector3(0, 0, 0), Quat(0, 1, 0, 0), Vector2(1, 1), size);
  EXPECT_TRUE(is_image_valid_eq(orbital_image.image(), patch, 1e-6));
}
