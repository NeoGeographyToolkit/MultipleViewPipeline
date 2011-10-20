#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/OrbitalImageCrop.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace mvp;

TEST(OrbtialImageCrop, crop) {
  // TODO: Actually test the cropping ability
  OrbitalImageFileDescriptor image_file;

  image_file.set_camera_path(SrcName("synth.0.pinhole"));
  image_file.set_image_path(SrcName("synth.0.tif"));

  OrbitalImageCrop orbitalimagecrop(image_file, BBox2());

  EXPECT_EQ(orbitalimagecrop.image().cols(), 460);
  EXPECT_EQ(orbitalimagecrop.image().rows(), 460);
}
