#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/OrbitalImageFootprint.h>

#include <vw/Image.h>
#include <vw/Camera.h>
#include <vw/FileIO.h>
#include <vw/Cartography.h>

using namespace std;
using namespace mvp;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;

TEST(OrbitalImageFootprint, backproj_px) {
  PinholeModel cam(SrcName("synth.0.pinhole"));

  Vector2 px_pick(100.0, 200.0);

  Vector2 lonlat = OrbitalImageFootprint::backproj_px(cam, px_pick, Datum("D_MOON"), 0);

  Vector3 llr(lonlat[0], lonlat[1], Datum("D_MOON").semi_major_axis());
  Vector3 xyz = vw::cartography::lon_lat_radius_to_xyz(llr);

  Vector2 px_pick_again = cam.point_to_pixel(xyz);

  EXPECT_VECTOR_NEAR(px_pick, px_pick_again, 1e-6);
}

ImageView<bool> rasterize_footprint(OrbitalImageFootprint const& image_fp, GeoReference const& georef, int cols, int rows) 
{

  ImageView<bool> img = constant_view(true, cols, rows);

  for (int j = 0; j < rows; j++) {
    for (int i = 0; i < cols; i++) {
      if (!image_fp.contains(georef.pixel_to_lonlat(Vector2(i, j)))) {
        img(i, j) = false;
      }
    }
  }

  return img;
}

ImageView<bool> rasterize_footprint_ref(OrbitalImageFootprint const& image_fp, GeoReference const& georef, int cols, int rows, Vector2 alt_range) 
{
  VW_ASSERT(georef.datum().semi_major_axis() == georef.datum().semi_minor_axis(), LogicErr() << "Spheroid datums not supported");

  Vector2 radius_range(alt_range + Vector2(georef.datum().semi_major_axis(), georef.datum().semi_major_axis()));

  boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_fp.orbital_image_file().image_path()));
  BBox2 image_bounds(0, 0, rsrc->cols(), rsrc->rows());

  ImageView<bool> img = constant_view(false, cols, rows);

  PinholeModel cam(image_fp.orbital_image_file().camera_path());

  const int num_checks = 20;

  for (int j = 0; j < rows; j++) {
    for (int i = 0; i < cols; i++) {
      Vector2 ll = georef.pixel_to_lonlat(Vector2(i, j));
      for (int k = 0; k < num_checks; k++) {
        double curr_rad = (radius_range[1] - radius_range[0]) * k / num_checks + radius_range[0];

        Vector3 llr(ll[0], ll[1], curr_rad);
        Vector3 xyz(lon_lat_radius_to_xyz(llr));
        Vector2 px(cam.point_to_pixel(xyz));

        if (image_bounds.contains(px)) {
          img(i, j) = true;
          break;
        }
      }
    }
  }

  return img;
}

TEST(OrbitalImageFootprint, footprint) {
  const int cols = 100, rows = 100;
  Vector2 alt_range(-10000, 10000);
  Datum datum("D_MOON");

  OrbitalImageFootprint image_fp(OrbitalImageFootprint::construct_from_paths(SrcName("synth.0.tif"), 
                                                                             SrcName("synth.0.pinhole"), 
                                                                             datum, alt_range));

  BBox2 bbox(image_fp.bounding_box());
  
  Matrix3x3 trans;
  trans.set_identity();
  trans(0, 0) = bbox.width() / cols;
  trans(1, 1) = -bbox.height() / rows;
  trans(0, 2) = bbox.min().x();
  trans(1, 2) = bbox.max().y();

  GeoReference georef(datum, trans, GeoReference::PixelAsPoint);

  ImageView<bool> result = rasterize_footprint(image_fp, georef, cols, rows);
  ImageView<bool> result_ref = rasterize_footprint_ref(image_fp, georef, cols, rows, alt_range);
  ImageView<bool> diff = apply_mask(intersect_mask(create_mask(result), invert_mask(create_mask(result_ref))));

  //write_image("result.tif", channel_cast_rescale<uint8>(result));
  //write_image("result_ref.tif", channel_cast_rescale<uint8>(result_ref));
  //write_image("diff.tif", channel_cast_rescale<uint8>(diff));

  double diff_ratio = double(sum_of_channel_values(diff)) / cols / rows;

  EXPECT_LT(diff_ratio, 0.02);
}

TEST(OrbitalImageFootprint, properties) {
  OrbitalImageFootprint image_fp(OrbitalImageFootprint::construct_from_paths(SrcName("synth.0.tif"), 
                                                                             SrcName("synth.0.pinhole"), 
                                                                             Datum("D_MOON"), Vector2()));

  BBox2 bbox(image_fp.bounding_box());
  EXPECT_VECTOR_NEAR(bbox.min(), Vector2(56.1308, 9.18129), 1e-3);
  EXPECT_VECTOR_NEAR(bbox.max(), Vector2(56.7642, 9.80684), 1e-3);

  EXPECT_EQ(image_fp.equal_resolution_level(), 9);

  EXPECT_EQ(image_fp.equal_density_level(256), 11);
}

TEST(OrbitalImageFootprint, intersects) {
  OrbitalImageFootprint image_fp(OrbitalImageFootprint::construct_from_paths(SrcName("synth.0.tif"), 
                                                                             SrcName("synth.0.pinhole"), 
                                                                             Datum("D_MOON"), Vector2()));
  //      Vector2(56.3215,9.80684)
  //           -----------------
  //          /                  --------------
  //         /                                / Vector2(56.7642,9.61175)
  //        /                                / 
  //       /                                /
  //      -----------------                /
  //                        --------------
  //Vector2(56.1308,9.37433)
  //                              Vector2(56.5729,9.18129)

  EXPECT_TRUE(image_fp.intersects(BBox2(56, 9, 0.5, 0.5)));
  EXPECT_FALSE(image_fp.intersects(BBox2(56, 9, 0.1, 0.1)));
}

TEST(OrbitalImageFootprintCollection, add_image) {
  OrbitalImageFootprintCollection images(Datum("D_MOON"), Vector2());

  images.add_image(SrcName("synth.0.tif"), SrcName("synth.0.pinhole"));

  EXPECT_EQ(images.size(), 1u);
  EXPECT_EQ(images.equal_resolution_level(), 9);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(56.1308, 9.18129), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(56.7642, 9.80684), 1e-3);

  images.add_image(SrcName("synth.1.tif"), SrcName("synth.1.pinhole"));
  images.add_image(SrcName("synth.2.tif"), SrcName("synth.2.pinhole"));
  images.add_image(SrcName("synth.3.tif"), SrcName("synth.3.pinhole"));

  EXPECT_EQ(images.size(), 4u);
  EXPECT_EQ(images.equal_resolution_level(), 9);
  EXPECT_EQ(images.equal_density_level(256), 11);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().min(), Vector2(56.0486, 9.18129), 1e-3);
  EXPECT_VECTOR_NEAR(images.lonlat_bbox().max(), Vector2(56.7642, 9.84013), 1e-3);
}

TEST(OrbitalImageFootprintCollection, images_in_region) {
  //TODO: Test me!!
}
