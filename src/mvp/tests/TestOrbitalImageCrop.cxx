#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/OrbitalImageCrop.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace mvp;

TEST(HelperFunction, offset_pinhole) {
  PinholeModel cam(SrcName("synth.0.pinhole"));
  PinholeModel cam_off(offset_pinhole(cam, Vector2(10, 20)));
 
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(10, 20)), cam_off.pixel_to_vector(Vector2(0, 0)), 1e-6);
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(20, 30)), cam_off.pixel_to_vector(Vector2(10, 10)), 1e-6);
}

TEST(OrbtialImageCrop, crop) {
  // TODO: Actually test the cropping ability
  OrbitalImageFileDescriptor image_file;

  image_file.set_camera_path(SrcName("synth.0.pinhole"));
  image_file.set_image_path(SrcName("synth.0.tif"));

  OrbitalImageCrop orbitalimagecrop(image_file, BBox2());

  EXPECT_EQ(orbitalimagecrop.image().cols(), 460);
  EXPECT_EQ(orbitalimagecrop.image().rows(), 460);
}
