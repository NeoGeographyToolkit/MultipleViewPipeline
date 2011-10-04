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
