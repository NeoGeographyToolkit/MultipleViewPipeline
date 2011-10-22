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

TEST(HelperFunction, backproj_px) {
  const double moon_rad = 1737400;

  PinholeModel cam(SrcName("AS15-M-0073.lev1.pinhole"));

  Vector2 px_pick(100.0, 200.0);

  Vector2 lonlat = backproj_px(cam, px_pick, moon_rad);

  Vector3 llr(lonlat[0], lonlat[1], moon_rad);
  Vector3 xyz = vw::cartography::lon_lat_radius_to_xyz(llr);

  Vector2 px_pick_again = cam.point_to_pixel(xyz);

  EXPECT_VECTOR_NEAR(px_pick, px_pick_again, 1e-6);
}

TEST(HelperFunction, isec_poly) {
  vector<Vector2> poly1, poly2, poly3, poly4;

  poly1.push_back(Vector2(30, 40));
  poly1.push_back(Vector2(30, 80));
  poly1.push_back(Vector2(50, 80));
  poly1.push_back(Vector2(50, 40));

  poly2.push_back(Vector2(40, 30));
  poly2.push_back(Vector2(40, 70));
  poly2.push_back(Vector2(60, 70));
  poly2.push_back(Vector2(60, 30));

  poly3.push_back(Vector2(45, 50));
  poly3.push_back(Vector2(45, 60));
  poly3.push_back(Vector2(47, 60));
  poly3.push_back(Vector2(47, 50));

  poly4.push_back(Vector2(48, 20));
  poly4.push_back(Vector2(48, 60));
  poly4.push_back(Vector2(70, 60));
  poly4.push_back(Vector2(70, 20));

  EXPECT_TRUE(isect_poly(poly1, poly2));
  EXPECT_TRUE(isect_poly(poly1, poly3));
  EXPECT_TRUE(isect_poly(poly2, poly3));
  EXPECT_TRUE(isect_poly(poly1, poly4));
  EXPECT_TRUE(isect_poly(poly2, poly4));
  EXPECT_FALSE(isect_poly(poly3, poly4));
}

TEST(OrbitalImageFootprint, construct_footprint) {
  vector<Vector2> fp(OrbitalImageFootprint::construct_footprint(PinholeModel(SrcName("AS15-M-0073.lev1.pinhole")), Vector2(5725, 5725), Vector2(1737400, 1737400)));

  EXPECT_EQ(fp.size(), 4);
  EXPECT_VECTOR_NEAR(fp[0], Vector2(173.541,-21.7811), 1e-3);
  EXPECT_VECTOR_NEAR(fp[1], Vector2(179.133,-22.5502), 1e-3);
  EXPECT_VECTOR_NEAR(fp[2], Vector2(178.42,-27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(fp[3], Vector2(172.639,-26.9718), 1e-3);
}

TEST(OrbitalImageFootprint, lonlat_bbox) {
  OrbitalImageFootprint image_fp(SrcName("AS15-M-0073.lev1.pinhole"), SrcName("dummy_image.73.png"), Datum("D_MOON"), Vector2(0, 0));

  BBox2 bbox(image_fp.lonlat_bbox());

  EXPECT_VECTOR_NEAR(bbox.min(), Vector2(172.639, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(bbox.max(), Vector2(179.133, -21.7811), 1e-3);
}

TEST(OrbitalImageFootprint, equal_resolution_level) {
  OrbitalImageFootprint image_fp(SrcName("AS15-M-0073.lev1.pinhole"), SrcName("dummy_image.73.png"), Datum("D_MOON"), Vector2(0, 0));

  EXPECT_EQ(image_fp.equal_resolution_level(), 6);
}

TEST(OrbitalImageFootprint, equal_density_level) {
  OrbitalImageFootprint image_fp(SrcName("AS15-M-0073.lev1.pinhole"), SrcName("dummy_image.73.png"), Datum("D_MOON"), Vector2(0, 0));

  EXPECT_EQ(image_fp.equal_density_level(256), 11);
}

TEST(OrbitalImageFootprint, intersects) {
  OrbitalImageFootprint image_fp(SrcName("AS15-M-0073.lev1.pinhole"), SrcName("dummy_image.73.png"), Datum("D_MOON"), Vector2(0, 0));

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

  images.add_image(SrcName("AS15-M-0073.lev1.pinhole"), SrcName("dummy_image.73.png"));

  EXPECT_EQ(images.size(), 1);
  EXPECT_EQ(images.equal_resolution_level(), 6);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(172.639, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(179.133, -21.7811), 1e-3);

  images.add_image(SrcName("AS15-M-0074.lev1.pinhole"), SrcName("dummy_image.73.png"));
  images.add_image(SrcName("AS15-M-0075.lev1.pinhole"), SrcName("dummy_image.73.png"));
  images.add_image(SrcName("AS15-M-0076.lev1.pinhole"), SrcName("dummy_image.73.png"));

  EXPECT_EQ(images.size(), 4);
  EXPECT_EQ(images.equal_resolution_level(), 6);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(179.133, -21.3673), 1e-3);
}

TEST(OrbitalImageFootprintCollection, add_image_pattern) {
  OrbitalImageFootprintCollection images(Datum("D_MOON"), Vector2(0, 0));
  images.add_image_pattern(SrcName("AS15-M-%04d.lev1.pinhole"), SrcName("dummy_image.%d.png"), 73, 76);

  EXPECT_EQ(images.size(), 4);
  EXPECT_EQ(images.equal_resolution_level(), 6);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(179.133, -21.3673), 1e-3);
}

TEST(OrbitalImageFootprintCollection, images_in_region) {
  //TODO: Test me!!
}
