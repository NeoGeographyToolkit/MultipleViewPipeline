#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/MVPWorkspace.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace mvp;

TEST(MVPWorkspace, work_area) {
  MVPWorkspace work("", "", PlateGeoReference(Datum("D_MOON")), MVPAlgorithmSettings());
  work.add_image_pattern(SrcName("AS15-M-%04d.lev1.pinhole"), SrcName("dummy_image.%d.png"), Vector2i(73, 76));

  EXPECT_VECTOR_NEAR(work.lonlat_work_area().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(work.lonlat_work_area().max(), Vector2(179.133, -21.3673), 1e-3);

  EXPECT_VECTOR_EQ(work.pixel_work_area(0).min(), Vector2i(247, 142));
  EXPECT_VECTOR_EQ(work.pixel_work_area(0).max(), Vector2i(255, 148));
  EXPECT_VECTOR_EQ(work.pixel_work_area(10).min(), Vector2i(254318, 146630));
  EXPECT_VECTOR_EQ(work.pixel_work_area(10).max(), Vector2i(261512, 151222));

  EXPECT_VECTOR_EQ(work.tile_work_area(0).min(), Vector2i(0, 0));
  EXPECT_VECTOR_EQ(work.tile_work_area(0).max(), Vector2i(1, 1));
  EXPECT_VECTOR_EQ(work.tile_work_area(10).min(), Vector2i(993, 572));
  EXPECT_VECTOR_EQ(work.tile_work_area(10).max(), Vector2i(1022, 591));
}

TEST(MVPWorkspace, images_at_tile) {
  MVPWorkspace work("", "", PlateGeoReference(Datum("D_MOON")), MVPAlgorithmSettings());
  work.add_image_pattern(SrcName("AS15-M-%04d.lev1.pinhole"), SrcName("dummy_image.%d.png"), Vector2i(73, 76));

  vector<OrbitalImageFileDescriptor> images;
 
  images = work.images_at_tile(0, 0, 0);
  EXPECT_EQ(images.size(), 4);

  images = work.images_at_tile(1, 1, 1);
  EXPECT_EQ(images.size(), 4);

  images = work.images_at_tile(0, 0, 1);
  EXPECT_EQ(images.size(), 0);

  images = work.images_at_tile(996, 578, 10);
  EXPECT_EQ(images.size(), 1);
}

TEST(MVPWorkspace, assemble_job) {
  MVPWorkspace work("result", "internal", PlateGeoReference(Datum("D_MOON")), MVPAlgorithmSettings());
  work.add_image_pattern(SrcName("AS15-M-%04d.lev1.pinhole"), SrcName("dummy_image.%d.png"), Vector2i(73, 76));

  MVPJobRequest job;
  
  job = work.assemble_job(1, 1, 1);

  EXPECT_EQ(job.tile().col(), 1);
  EXPECT_EQ(job.tile().row(), 1);
  EXPECT_EQ(job.tile().level(), 1);
  EXPECT_EQ(job.result_platefile(), "result");
  EXPECT_EQ(job.internal_result_platefile(), "internal");
  EXPECT_EQ(job.plate_georef().DebugString(), PlateGeoReference(Datum("D_MOON")).build_desc().DebugString());
  EXPECT_EQ(job.algorithm_settings().DebugString(), MVPAlgorithmSettings().DebugString());
  EXPECT_EQ(job.orbital_images_size(), 4);
}
