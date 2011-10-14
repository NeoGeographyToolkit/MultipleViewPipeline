#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/OrbitalImageCrop.h>
#include <mvp/OrbitalImageFile.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace mvp;

TEST(OrbtialImageCrop, constructors) {
  string camera_file(SrcName("AS15-M-0073.lev1.pinhole"));
  string image_file(SrcName("dummy_image.73.png"));

  OrbitalImageFile orbitalimagefile(camera_file, image_file, Vector2(1737400, 1737400));

  OrbitalImageCrop imgcrop(orbitalimagefile.build_desc(), BBox2());
}

TEST(OrbitalImageCrop, crop) {
  // TODO
}
