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

TEST_F(OrbitalImageTest, Intersects) {
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

//  EXPECT_TRUE(orbimg.intersects(0, 0, 0));
//  EXPECT_FALSE(orbimg.intersects(0, 0, 20));

//  EXPECT_TRUE(orbimg.intersects(63, 36, 6));
//  EXPECT_FALSE(orbimg.intersects(63, 37, 6));
}
