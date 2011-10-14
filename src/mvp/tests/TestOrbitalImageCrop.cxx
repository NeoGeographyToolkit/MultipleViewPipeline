#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/OrbitalImageCrop.h>
#include <mvp/OrbitalImageFile.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace mvp;

TEST(OrbtialImageCrop, crop) {
  // TODO: Actually test the cropping ability
  string camera_file(SrcName("synth.0.pinhole"));
  string image_file(SrcName("synth.0.tif"));

  OrbitalImageFile orbitalimagefile(camera_file, image_file, Vector2(1737400, 1737400));

  OrbitalImageCrop orbitalimagecrop(orbitalimagefile.build_desc(), BBox2());

  EXPECT_EQ(orbitalimagecrop.image().cols(), 460);
  EXPECT_EQ(orbitalimagecrop.image().rows(), 460);
}
