#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/PolygonMath.h>

using namespace std;
using namespace mvp;
using namespace vw;
using namespace vw::camera;
using namespace vw::cartography;
using namespace vw::test;

TEST(PolygonMath, backproj_px) {
  PinholeModel cam(SrcName("AS15-M-0073.lev1.pinhole"));

  Vector2 px_pick(100.0, 200.0);

  Vector2 lonlat = backproj_px(cam, px_pick, Datum("D_MOON"), 0);

  Vector3 llr(lonlat[0], lonlat[1], Datum("D_MOON").semi_major_axis());
  Vector3 xyz = vw::cartography::lon_lat_radius_to_xyz(llr);

  Vector2 px_pick_again = cam.point_to_pixel(xyz);

  EXPECT_VECTOR_NEAR(px_pick, px_pick_again, 1e-6);
}

TEST(PolygonMath, isec_poly) {
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

TEST(PolygonMath, circulation_direction) {
  EXPECT_GT(circulation_direction(Vector2(2, 2), Vector2(3, 3), Vector2(1, 5)), 0);
  EXPECT_LT(circulation_direction(Vector2(1, 5), Vector2(3, 3), Vector2(2, 2)), 0);
  EXPECT_EQ(circulation_direction(Vector2(2, 2), Vector2(3, 3), Vector2(4, 4)), 0);
}
