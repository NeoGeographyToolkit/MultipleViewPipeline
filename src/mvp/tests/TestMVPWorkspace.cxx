#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/MVPWorkspace.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace mvp;

class MVPWorkspaceTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
      PlateGeoReference plate_georef(Datum("D_MOON"));

      work_request.set_result_platefile("result");
      work_request.set_internal_result_platefile("internal");
      *work_request.mutable_plate_georef() = plate_georef.build_desc();
      *work_request.mutable_user_settings() = MVPUserSettings();

      for (int i = 0; i <= 3; i++) {
        stringstream ss;
        ss << "synth." << i;
        OrbitalImageFileDescriptor image;
        image.set_image_path(SrcName(ss.str() + ".tif"));
        image.set_camera_path(SrcName(ss.str() + ".pinhole"));
        *work_request.add_orbital_images() = image;
      }
    }

    MVPWorkspaceRequest work_request;
}; 

TEST_F(MVPWorkspaceTest, render_defaults) {
  MVPWorkspace work(work_request);

  EXPECT_EQ(work.render_level(), 11);

  EXPECT_VECTOR_EQ(work.render_bbox().min(), Vector2i(1342, 968));
  EXPECT_VECTOR_EQ(work.render_bbox().max(), Vector2i(1347, 972));

  EXPECT_VECTOR_NEAR(work.render_lonlat_bbox().min(), Vector2(55.8988, 9.14028), 1e-3);
  EXPECT_VECTOR_NEAR(work.render_lonlat_bbox().max(), Vector2(56.7777, 9.84341), 1e-3);

  EXPECT_VECTOR_NEAR(work.footprints().lonlat_bbox().min(), Vector2(56.0486, 9.18129), 1e-3);
  EXPECT_VECTOR_NEAR(work.footprints().lonlat_bbox().max(), Vector2(56.7642, 9.84013), 1e-3);
}

TEST_F(MVPWorkspaceTest, images_at_tile) {
  MVPWorkspace work(work_request);
  
  vector<OrbitalImageFileDescriptor> images;
 
  images = work.images_at_tile(0, 0, 0);
  EXPECT_EQ(images.size(), 4u);

  images = work.images_at_tile(1, 1, 1);
  EXPECT_EQ(images.size(), 4u);

  images = work.images_at_tile(0, 0, 1);
  EXPECT_EQ(images.size(), 0u);

  // Same as tile in TestMVPAlgorithm. TODO: make standard test tile?
  images = work.images_at_tile(5374, 3875, 13);
  EXPECT_EQ(images.size(), 2u);
}

TEST_F(MVPWorkspaceTest, assemble_job) {
  MVPWorkspace work(work_request);

  MVPJobRequest job;
  
  job = work.assemble_job(1, 1, 1);

  EXPECT_EQ(job.col(), 1);
  EXPECT_EQ(job.row(), 1);
  EXPECT_EQ(job.level(), 1);
  // TODO: check tile_size?
  EXPECT_EQ(job.result_platefile(), "result");
  EXPECT_EQ(job.internal_result_platefile(), "internal");
  // TODO: check georef?
  EXPECT_EQ(job.user_settings().DebugString(), MVPUserSettings().DebugString());
  EXPECT_EQ(job.orbital_images_size(), 4);
}
