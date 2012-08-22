#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/OrbitalImage/Footprint.h>

#include <vw/Image.h>
#include <vw/Camera.h>
#include <vw/FileIO.h>
#include <vw/Cartography.h>

using namespace std;
using namespace mvp::orbitalimage;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;

TEST(Footprint, backproj_px) {
  PinholeModel cam(SrcName("data/synth.0.pinhole"));

  Vector2 px_pick(100.0, 200.0);

  Vector2 lonlat = Footprint::backproj_px(cam, px_pick, Datum("D_MOON"), 0);

  Vector3 llr(lonlat[0], lonlat[1], Datum("D_MOON").semi_major_axis());
  Vector3 xyz = vw::cartography::lon_lat_radius_to_xyz(llr);

  Vector2 px_pick_again = cam.point_to_pixel(xyz);

  EXPECT_VECTOR_NEAR(px_pick, px_pick_again, 1e-6);
}

ImageView<bool> rasterize_footprint(Footprint const& footprint, GeoReference const& georef, int cols, int rows) 
{

  ImageView<bool> img = constant_view(true, cols, rows);

  for (int j = 0; j < rows; j++) {
    for (int i = 0; i < cols; i++) {
      if (!footprint.contains(georef.pixel_to_lonlat(Vector2(i, j)))) {
        img(i, j) = false;
      }
    }
  }

  return img;
}

ImageView<bool> rasterize_footprint_ref(Footprint const& footprint, GeoReference const& georef, int cols, int rows, Vector2 alt_range) 
{
  VW_ASSERT(georef.datum().semi_major_axis() == georef.datum().semi_minor_axis(), LogicErr() << "Spheroid datums not supported");

  Vector2 radius_range(alt_range + Vector2(georef.datum().semi_major_axis(), georef.datum().semi_major_axis()));

  boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(footprint.orbital_image().image_path()));
  BBox2 image_bounds(0, 0, rsrc->cols(), rsrc->rows());

  ImageView<bool> img = constant_view(false, cols, rows);

  PinholeModel cam(footprint.orbital_image().camera_path());

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

TEST(Footprint, footprint) {
  const int cols = 100, rows = 100;
  Vector2 alt_range(-10000, 10000);
  Datum datum("D_MOON");

  Footprint footprint(Footprint::construct_from_paths(SrcName("data/synth.0.tif"), 
                                                      SrcName("data/synth.0.pinhole"), 
                                                      datum, alt_range));

  BBox2 bbox(footprint.bounding_box());
  
  Matrix3x3 trans;
  trans.set_identity();
  trans(0, 0) = bbox.width() / cols;
  trans(1, 1) = -bbox.height() / rows;
  trans(0, 2) = bbox.min().x();
  trans(1, 2) = bbox.max().y();

  GeoReference georef(datum, trans, GeoReference::PixelAsPoint);

  ImageView<bool> result = rasterize_footprint(footprint, georef, cols, rows);
  ImageView<bool> result_ref = rasterize_footprint_ref(footprint, georef, cols, rows, alt_range);
  ImageView<bool> diff = apply_mask(intersect_mask(create_mask(result), invert_mask(create_mask(result_ref))));

  //write_image("result.tif", channel_cast_rescale<uint8>(result));
  //write_image("result_ref.tif", channel_cast_rescale<uint8>(result_ref));
  //write_image("diff.tif", channel_cast_rescale<uint8>(diff));

  double diff_ratio = double(sum_of_channel_values(diff)) / cols / rows;

  EXPECT_LT(diff_ratio, 0.02);
}

TEST(Footprint, properties) {
  Footprint footprint(Footprint::construct_from_paths(SrcName("data/synth.0.tif"), 
                                                      SrcName("data/synth.0.pinhole"), 
                                                      Datum("D_MOON"), Vector2()));

  BBox2 bbox(footprint.bounding_box());
  EXPECT_VECTOR_NEAR(bbox.min(), Vector2(56.1308, 9.18129), 1e-3);
  EXPECT_VECTOR_NEAR(bbox.max(), Vector2(56.7642, 9.80684), 1e-3);

  EXPECT_EQ(footprint.equal_resolution_level(), 9);

  EXPECT_EQ(footprint.equal_density_level(256), 11);
}

TEST(Footprint, intersects) {
  Footprint footprint(Footprint::construct_from_paths(SrcName("data/synth.0.tif"), 
                                                      SrcName("data/synth.0.pinhole"), 
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

  EXPECT_TRUE(footprint.intersects(BBox2(56, 9, 0.5, 0.5)));
  EXPECT_FALSE(footprint.intersects(BBox2(56, 9, 0.1, 0.1)));
}
