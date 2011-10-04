#include <gtest/gtest.h>
#include <test/Helpers.h>

#include "MVPWorkspace.h"

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace mvp;

TEST(MVPWorkspace, AddImage) {
  MVPWorkspace work(PlateGeoReference(Datum("D_MOON")), MVPOperationDesc(), Vector2());
  work.add_image(SrcName("AS15-M-0073.lev1.pinhole"), SrcName("dummy_image.tif"));

  EXPECT_EQ(work.num_images(), 1);
  EXPECT_EQ(work.equal_resolution_level(), 6);
  EXPECT_EQ(work.equal_density_level(), 11);
  EXPECT_VECTOR_NEAR(work.lonlat_work_area().min(), Vector2(172.639, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(work.lonlat_work_area().max(), Vector2(179.133, -21.7811), 1e-3);

  work.add_image(SrcName("AS15-M-0074.lev1.pinhole"), SrcName("dummy_image.tif"));
  work.add_image(SrcName("AS15-M-0075.lev1.pinhole"), SrcName("dummy_image.tif"));
  work.add_image(SrcName("AS15-M-0076.lev1.pinhole"), SrcName("dummy_image.tif"));

  EXPECT_EQ(work.num_images(), 4);
  EXPECT_EQ(work.equal_resolution_level(), 6);
  EXPECT_EQ(work.equal_density_level(), 11);
  EXPECT_VECTOR_NEAR(work.lonlat_work_area().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(work.lonlat_work_area().max(), Vector2(179.133, -21.3673), 1e-3);
}


TEST(MVPWorkspace, AddImagePattern) {
  MVPWorkspace work(PlateGeoReference(Datum("D_MOON")), MVPOperationDesc(), Vector2());
  work.add_image_pattern(SrcName("AS15-M-%04d.lev1.pinhole"), SrcName("dummy_image.%d.png"), Vector2i(73, 76));

  EXPECT_EQ(work.num_images(), 4);
  EXPECT_EQ(work.equal_resolution_level(), 6);
  EXPECT_EQ(work.equal_density_level(), 11);
  EXPECT_VECTOR_NEAR(work.lonlat_work_area().min(), Vector2(169.254, -27.6722), 1e-3);
  EXPECT_VECTOR_NEAR(work.lonlat_work_area().max(), Vector2(179.133, -21.3673), 1e-3);
}

TEST(MVPWorkspace, WorkAreas) {
  MVPWorkspace work(PlateGeoReference(Datum("D_MOON")), MVPOperationDesc(), Vector2());
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

TEST(MVPWorkspace, ImagesAtTile) {
  MVPWorkspace work(PlateGeoReference(Datum("D_MOON")), MVPOperationDesc(), Vector2());
  work.add_image_pattern(SrcName("AS15-M-%04d.lev1.pinhole"), SrcName("dummy_image.%d.png"), Vector2i(73, 76));

  OrbitalImageCollection collect;
 
  collect = work.images_at_tile(0, 0, 0);
  EXPECT_EQ(collect.size(), 4);

  collect = work.images_at_tile(1, 1, 1);
  EXPECT_EQ(collect.size(), 4);

  collect = work.images_at_tile(0, 0, 1);
  EXPECT_EQ(collect.size(), 0);

  collect = work.images_at_tile(996, 578, 10);
  EXPECT_EQ(collect.size(), 1);
}
