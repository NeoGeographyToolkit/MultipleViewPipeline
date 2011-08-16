#include "orbitfoot.h"
#include "gtest/gtest.h"

#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace fs = boost::filesystem;

//TODO: Set this define with cmake instead
#define TEST_SRCDIR "."
#include "Helpers.h"

class OrbitFootprintTest : public ::testing::Test {
  private:
    std::string create_camfile(vw::camera::PinholeModel cam) {
      static int num = 0;
      std::ostringstream oss;
      oss << TEST_SRCDIR << "/cam" << num << ".pinhole";
      cam.write(oss.str());
      num++;
      return oss.str();
    }

  protected:
    virtual void SetUp() {
      moon_rad = 1737400;

      // AS15-M-0073.lev1.pinhole
      camfiles.push_back(create_camfile(
        vw::camera::PinholeModel(vw::Vector3(-1664843.82,116630.703,-771606.935),
        vw::Quat(0.6196358355,-0.3398896742,0.413973702,-0.5737178878).rotation_matrix(),
        3802.7,3802.7,2862.875,2862.875,
        vw::Vector3(1,0,0),
        vw::Vector3(0,1,0),
        vw::Vector3(0,0,1),
        vw::camera::NullLensDistortion())
        ));

      // AS15-M-0074.lev1.pinhole
      camfiles.push_back(create_camfile(
        vw::camera::PinholeModel(vw::Vector3(-1663792.135,149562.261,-766972.8984),
        vw::Quat(0.6148602595,-0.3361016238,0.4200083161,-0.5766936572).rotation_matrix(),
        3802.7,3802.7,2862.875,2862.875,
        vw::Vector3(1,0,0),
        vw::Vector3(0,1,0),
        vw::Vector3(0,0,1),
        vw::camera::NullLensDistortion())
        ));

      // AS15-M-0075.lev1.pinhole
      camfiles.push_back(create_camfile(
        vw::camera::PinholeModel(vw::Vector3(-1662276.218,182135.2815,-762473.6572),
        vw::Quat(0.609961965,-0.33242215,0.4260734901,-0.5795716491).rotation_matrix(),
        3802.7,3802.7,2862.875,2862.875,
        vw::Vector3(1,0,0),
        vw::Vector3(0,1,0),
        vw::Vector3(0,0,1),
        vw::camera::NullLensDistortion())
        ));
    }

    virtual void TearDown() {
      BOOST_FOREACH(std::string file, camfiles) {
        fs::remove(file);
      }
    }

    double moon_rad;
    std::vector<std::string> camfiles;
};

TEST_F(OrbitFootprintTest, BackprojPx) {
  using namespace vw;

  vw::camera::PinholeModel cam(camfiles[0]);

  Vector2 px_pick(100.0, 200.0);

  Vector2 lonlat = backproj_px(cam, px_pick, moon_rad);

  Vector3 llr(lonlat[0], lonlat[1], moon_rad);
  Vector3 xyz = vw::cartography::lon_lat_radius_to_xyz(llr);

  Vector2 px_pick_again = cam.point_to_pixel(xyz);

  EXPECT_VECTOR_NEAR(px_pick, px_pick_again, 1e-6);
}

TEST_F(OrbitFootprintTest, Intersects) {
  using namespace vw;
  
  OrbitFootprint ofp(camfiles[0], Vector2i(5725, 5725), moon_rad);
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

  EXPECT_TRUE(ofp.intersects(BBox2()));
}
