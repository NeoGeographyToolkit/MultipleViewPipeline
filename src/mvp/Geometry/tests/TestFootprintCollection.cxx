#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/Geometry/FootprintCollection.h>

#include <vw/Image.h>
#include <vw/Camera.h>
#include <vw/FileIO.h>
#include <vw/Cartography.h>

using namespace std;
using namespace mvp::geometry;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;

TEST(FootprintCollection, push_back) {
  FootprintCollection footprints(Datum("D_MOON"), Vector2());

  footprints.push_back(DataName("synth.0.tif"), DataName("synth.0.pinhole"));

  EXPECT_EQ(footprints.size(), 1u);
  EXPECT_EQ(footprints.equal_resolution_level(), 9);
  EXPECT_EQ(footprints.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(footprints.lonlat_bbox().min(), Vector2(56.1308, 9.18129), 1e-3);
  EXPECT_VECTOR_NEAR(footprints.lonlat_bbox().max(), Vector2(56.7642, 9.80684), 1e-3);

  footprints.push_back(DataName("synth.1.tif"), DataName("synth.1.pinhole"));
  footprints.push_back(DataName("synth.2.tif"), DataName("synth.2.pinhole"));
  footprints.push_back(DataName("synth.3.tif"), DataName("synth.3.pinhole"));

  EXPECT_EQ(footprints.size(), 4u);
  EXPECT_EQ(footprints.equal_resolution_level(), 9);
  EXPECT_EQ(footprints.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(footprints.lonlat_bbox().min(), Vector2(56.0486, 9.18129), 1e-3);
  EXPECT_VECTOR_NEAR(footprints.lonlat_bbox().max(), Vector2(56.7642, 9.84013), 1e-3);
}

TEST(FootprintCollection, push_back_pattern) {
  // TODO: Test me!!
}

TEST(FootprintCollection, images_in_region) {
  //TODO: Test me!!
}
