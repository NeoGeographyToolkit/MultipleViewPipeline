#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/MVPWorkspace.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace mvp;

MVPWorkspaceRequest test_workspace() {
  PlateGeoReference plate_geo(Datum("D_MOON"));
  MVPUserSettings user_settings;

  MVPWorkspaceRequest work_request;
  work_request.set_result_platefile("result");
  work_request.set_internal_result_platefile("internal");
  *work_request.mutable_plate_georef() = plate_geo.build_desc();
  *work_request.mutable_user_settings() = user_settings;
  work_request.set_render_level(10);

  for (int i = 73; i <= 76; i++) {
    stringstream ss;
    ss << "AS15-M-00" << i << ".lev1.pinhole";
    OrbitalImageFileDescriptor image;
    image.set_image_path(SrcName("dummy_image.png"));
    image.set_camera_path(SrcName(ss.str()));
    *work_request.add_orbital_images() = image;
  }

  return work_request;
}


TEST(MVPWorkspace, render_defaults) {
  MVPWorkspace work(test_workspace());

  EXPECT_EQ(work.render_level(), 10);

  EXPECT_VECTOR_EQ(work.render_bbox().min(), Vector2i(993, 572));
  EXPECT_VECTOR_EQ(work.render_bbox().max(), Vector2i(1022, 591));

  EXPECT_VECTOR_NEAR(work.render_lonlat_bbox().min(), Vector2(169.102, -27.7741), 1e-3);
  EXPECT_VECTOR_NEAR(work.render_lonlat_bbox().max(), Vector2(179.298, -21.0944), 1e-3);

  EXPECT_VECTOR_NEAR(work.footprints().lonlat_bbox().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(work.footprints().lonlat_bbox().max(), Vector2(179.133, -21.3673), 1e-3);
}

TEST(MVPWorkspace, images_at_tile) {
  vector<OrbitalImageFileDescriptor> images;
  
  MVPWorkspace work(test_workspace());
 
  images = work.images_at_tile(0, 0, 0);
  EXPECT_EQ(images.size(), 4u);

  images = work.images_at_tile(1, 1, 1);
  EXPECT_EQ(images.size(), 4u);

  images = work.images_at_tile(0, 0, 1);
  EXPECT_EQ(images.size(), 0u);

  images = work.images_at_tile(996, 578, 10);
  EXPECT_EQ(images.size(), 1u);
}

TEST(MVPWorkspace, assemble_job) {
  MVPWorkspace work(test_workspace());

  MVPJobRequest job;
  
  job = work.assemble_job(1, 1, 1);

  EXPECT_EQ(job.col(), 1);
  EXPECT_EQ(job.row(), 1);
  EXPECT_EQ(job.level(), 1);
  EXPECT_EQ(job.result_platefile(), "result");
  EXPECT_EQ(job.internal_result_platefile(), "internal");
  EXPECT_EQ(job.plate_georef().DebugString(), PlateGeoReference(Datum("D_MOON")).build_desc().DebugString());
  EXPECT_EQ(job.user_settings().DebugString(), MVPUserSettings().DebugString());
  EXPECT_EQ(job.orbital_images_size(), 4);
}
