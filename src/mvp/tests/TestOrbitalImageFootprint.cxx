#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/OrbitalImageFootprint.h>

#include <vw/Camera.h>
#include <vw/FileIO.h>
#include <vw/Cartography.h>

using namespace std;
using namespace mvp;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;

TEST(OrbitalImageFootprint, construct_footprint) {
  vector<Vector2> fp(OrbitalImageFootprint::construct_footprint(PinholeModel(SrcName("AS15-M-0073.lev1.pinhole")), Vector2(5725, 5725), Vector2(1737400, 1737400)));

  EXPECT_EQ(fp.size(), 4);
  EXPECT_VECTOR_NEAR(fp[0], Vector2(173.541,-21.7811), 1e-3);
  EXPECT_VECTOR_NEAR(fp[1], Vector2(179.133,-22.5502), 1e-3);
  EXPECT_VECTOR_NEAR(fp[2], Vector2(178.42,-27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(fp[3], Vector2(172.639,-26.9718), 1e-3);
}

TEST(OrbitalImageFootprint, lonlat_bbox) {
  OrbitalImageFootprint image_fp(SrcName("dummy_image.73.png"), SrcName("AS15-M-0073.lev1.pinhole"), Datum("D_MOON"), Vector2(0, 0));

  BBox2 bbox(image_fp.lonlat_bbox());

  EXPECT_VECTOR_NEAR(bbox.min(), Vector2(172.639, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(bbox.max(), Vector2(179.133, -21.7811), 1e-3);
}

TEST(OrbitalImageFootprint, equal_resolution_level) {
  OrbitalImageFootprint image_fp(SrcName("dummy_image.73.png"), SrcName("AS15-M-0073.lev1.pinhole"), Datum("D_MOON"), Vector2(0, 0));

  EXPECT_EQ(image_fp.equal_resolution_level(), 6);
}

TEST(OrbitalImageFootprint, equal_density_level) {
  OrbitalImageFootprint image_fp(SrcName("dummy_image.73.png"), SrcName("AS15-M-0073.lev1.pinhole"), Datum("D_MOON"), Vector2(0, 0));

  EXPECT_EQ(image_fp.equal_density_level(256), 11);
}

TEST(OrbitalImageFootprint, intersects) {
  OrbitalImageFootprint image_fp(SrcName("dummy_image.73.png"), SrcName("AS15-M-0073.lev1.pinhole"), Datum("D_MOON"), Vector2(0, 0));

  // Vector2(173.541,-21.7811)
  //           -----------------
  //          /                  --------------
  //         /                                /    Vector2(179.133,-22.5502)
  //        /                                / 
  //       /                                /
  //      -----------------                /
  //                        --------------
  //Vector2(172.639,-26.9718)
  //                              Vector2(178.42,-27.6722)

  EXPECT_TRUE(image_fp.intersects(BBox2(175, -25, 5, 5)));
  EXPECT_FALSE(image_fp.intersects(BBox2(180, -25, 5, 5)));
}

TEST(OrbitalImageFootprintCollection, add_image) {
  OrbitalImageFootprintCollection images(Datum("D_MOON"), Vector2(0, 0));

  images.add_image(SrcName("dummy_image.73.png"), SrcName("AS15-M-0073.lev1.pinhole"));

  EXPECT_EQ(images.size(), 1);
  EXPECT_EQ(images.equal_resolution_level(), 6);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(172.639, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(179.133, -21.7811), 1e-3);

  images.add_image(SrcName("dummy_image.74.png"), SrcName("AS15-M-0074.lev1.pinhole"));
  images.add_image(SrcName("dummy_image.75.png"), SrcName("AS15-M-0075.lev1.pinhole"));
  images.add_image(SrcName("dummy_image.76.png"), SrcName("AS15-M-0076.lev1.pinhole"));

  EXPECT_EQ(images.size(), 4);
  EXPECT_EQ(images.equal_resolution_level(), 6);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(179.133, -21.3673), 1e-3);
}

TEST(OrbitalImageFootprintCollection, add_image_pattern) {
  OrbitalImageFootprintCollection images(Datum("D_MOON"), Vector2(0, 0));
  images.add_image_pattern(SrcName("dummy_image.%d.png"), SrcName("AS15-M-%04d.lev1.pinhole"), 73, 76);

  EXPECT_EQ(images.size(), 4);
  EXPECT_EQ(images.equal_resolution_level(), 6);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(179.133, -21.3673), 1e-3);
}

TEST(OrbitalImageFootprintCollection, images_in_region) {
  //TODO: Test me!!
}
