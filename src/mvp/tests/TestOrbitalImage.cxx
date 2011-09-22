#include <gtest/gtest.h>
#include <test/Helpers.h>

#include "OrbitalImage.h"

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
  double moon_rad = 1737400;

  PinholeModel cam(vw::Vector3(-1664843.82,116630.703,-771606.935),
    vw::Quat(0.6196358355,-0.3398896742,0.413973702,-0.5737178878).rotation_matrix(),
    3802.7,3802.7,2862.875,2862.875,
    vw::Vector3(1,0,0),
    vw::Vector3(0,1,0),
    vw::Vector3(0,0,1),
    vw::camera::NullLensDistortion());

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

class OrbitalImageTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
      camfile = UnlinkName("cam.pinhole");
      imgsize = Vector2i(5725, 5725);
      radius_range = Vector2(1737400, 1737400);

      // AS15-M-0073.lev1.pinhole
      PinholeModel cam(vw::Vector3(-1664843.82,116630.703,-771606.935),
        vw::Quat(0.6196358355,-0.3398896742,0.413973702,-0.5737178878).rotation_matrix(),
        3802.7,3802.7,2862.875,2862.875,
        vw::Vector3(1,0,0),
        vw::Vector3(0,1,0),
        vw::Vector3(0,0,1),
        vw::camera::NullLensDistortion());
      
      cam.write(camfile);
    }

    UnlinkName camfile;
    Vector2i imgsize;
    Vector2 radius_range;
};

TEST_F(OrbitalImageTest, build_desc) {
  OrbitalImage orbimg(camfile, imgsize, radius_range);

  OrbitalImageDesc desc(orbimg.build_desc());

  OrbitalImage orbimg2(desc);

  OrbitalImageDesc desc2(orbimg2.build_desc());

  EXPECT_EQ(desc.DebugString(), desc2.DebugString());
}

TEST_F(OrbitalImageTest, construct_footprint) {
  vector<Vector2> fp(OrbitalImage::construct_footprint(PinholeModel(camfile), imgsize, radius_range));

  EXPECT_EQ(fp.size(), 4);
  EXPECT_VECTOR_NEAR(fp[0], Vector2(173.541,-21.7811), 1e-3);
  EXPECT_VECTOR_NEAR(fp[1], Vector2(179.133,-22.5502), 1e-3);
  EXPECT_VECTOR_NEAR(fp[2], Vector2(178.42,-27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(fp[3], Vector2(172.639,-26.9718), 1e-3);
}

TEST_F(OrbitalImageTest, footprint_bbox) {
  // TODO
}

TEST_F(OrbitalImageTest, set_radius_range) {
  Vector2 newrad(1736400, 1738400);
  OrbitalImage orbimg(camfile, imgsize, radius_range);
  OrbitalImage orbimg2(camfile, imgsize, newrad);

  orbimg.set_radius_range(newrad);
 
  EXPECT_EQ(orbimg.build_desc().DebugString(), orbimg2.build_desc().DebugString()); 
}

TEST_F(OrbitalImageTest, equal_resolution_level) {
  OrbitalImage orbimg(camfile, imgsize, radius_range);

  EXPECT_EQ(orbimg.equal_resolution_level(), 6);
}

TEST_F(OrbitalImageTest, equal_density_level) {
  OrbitalImage orbimg(camfile, imgsize, radius_range);

  EXPECT_EQ(orbimg.equal_density_level(), 20);
}

TEST_F(OrbitalImageTest, intersects) {
  OrbitalImage orbimg(camfile, imgsize, radius_range);

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

  EXPECT_TRUE(orbimg.intersects(BBox2(175, -25, 5, 5)));
  EXPECT_FALSE(orbimg.intersects(BBox2(180, -25, 5, 5)));
}
