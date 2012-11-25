#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/Geometry/FootprintProjector.h>

#include <vw/Camera.h>

using namespace std;
using namespace mvp::geometry;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;

TEST(FootprintProjector, project_pt) {
  boost::shared_ptr<CameraModel> camera(new PinholeModel(DataName("synth.0.pinhole")));
  Datum datum("D_MOON");

  FootprintProjector fp(camera, datum);

  Vector2 pick(100, 200);

  EXPECT_VECTOR_NEAR(fp.forward(fp.backward(pick, 0), 0), pick, 1e-6);
  EXPECT_VECTOR_NEAR(fp.forward(fp.backward(pick, 100), 100), pick, 1e-6);
}
