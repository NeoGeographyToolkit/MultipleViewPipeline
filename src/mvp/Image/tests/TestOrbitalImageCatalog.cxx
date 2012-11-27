#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Image/OrbitalImageCatalog.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;
using namespace mvp::image;

TEST(OrbitalImageCatalog, add_image_pattern) {

  OrbitalImageCatalog catalog(Datum("D_MOON"), Vector2i());

  catalog.add_image_pattern(DataName("synth\\.\\d*\\.tif"), DataName("synth\\.\\d*\\.pinhole"));

  EXPECT_EQ(catalog.size(), 4);
}

TEST(OrbitalImageCatalog, images_in_region) {

  // TODO: Write me

}
