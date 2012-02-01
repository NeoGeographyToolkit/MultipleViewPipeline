#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/OrbitalImageCrop.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;
using namespace mvp;

TEST(HelperFunction, crop_pinhole) {
  PinholeModel cam(SrcName("synth.0.pinhole"));
  PinholeModel crop_cam(crop(cam, BBox2i(10, 20, 100, 100)));
 
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(10, 20)), crop_cam.pixel_to_vector(Vector2(0, 0)), 1e-6);
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(20, 30)), crop_cam.pixel_to_vector(Vector2(10, 10)), 1e-6);
}

TEST(OrbtialImageCrop, construct_from_paths) {
/* Footprint for synth.0.pinhole with alt +-10000

    Vector2(56.1474,9.90343)
                                           Vector2(56.6319,9.69012)
                                                                  
                                           
                                           
                                                                      Vector2(56.8948,9.53431)

Vector2(55.9392,9.4308)





                                 Vector2(56.4232,9.21995)
                                                                  Vector2(56.7208,9.14307)
*/
  // Number of points to test per range
  const int num_pts = 20;

  // Set up test space
  string image_path(SrcName("synth.0.tif"));
  string camera_path(SrcName("synth.0.pinhole"));
  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);
  Vector2 radius_range(alt_range + Vector2(datum.semi_major_axis(), datum.semi_major_axis()));

  // Reference image, not cropped
  OrbitalImageCrop image_nocrop(OrbitalImageCrop::construct_from_paths(image_path, camera_path));

  // Bounding boxes to test
  vector<BBox2> boxes(4);
  boxes[0] = BBox2(55.9392, 9.21995, 0.1, 0.1); // outside footprint
  boxes[1] = BBox2(55.9392, 9.21995, 0.1, 0.3); // overlap footprint
  boxes[2] = BBox2(56.4232, 9.4308, 0.1, 0.1); // inside footprint
  boxes[3] = BBox2(55, 9, 2, 1); // entire footprint

  // For each bbox to test, break the lonlat and alt range into
  // num_pts points and make sure the crop and reference give
  // the same values
  BOOST_FOREACH(BBox2 const& b, boxes) {
    OrbitalImageCrop image_crop(OrbitalImageCrop::construct_from_paths(image_path, camera_path, b, datum, alt_range));

    for (int i = 0; i < num_pts; i++) {
      for (int j = 0; j < num_pts; j++) {
        for (int k = 0; k < num_pts; k++) {
          double lon = b.width() * i / num_pts + b.min().x();
          double lat = b.height() * j / num_pts + b.min().y();
          double rad = (radius_range[1] - radius_range[0]) * k / num_pts + radius_range[0];
          Vector3 llr(lon, lat, rad);
          Vector3 xyz(lon_lat_radius_to_xyz(llr));

          Vector2 px_crop(image_crop.camera().point_to_pixel(xyz));
          Vector2 px_nocrop(image_nocrop.camera().point_to_pixel(xyz));

          // If the uncropped image contains the pixel, then the crop MUST have it, and be the same value
          // If the uncropped image doesn't contain the pixel, the crop can have it or not.
          if (bounding_box(image_nocrop).contains(px_nocrop)) {
            ASSERT_TRUE(bounding_box(image_crop).contains(px_crop));
            EXPECT_TYPE_EQ(image_crop(px_crop.x(), px_crop.y()), image_nocrop(px_nocrop.x(), px_nocrop.y()));
          }
        }
      }
    }
  }

  OrbitalImageCrop empty_image(OrbitalImageCrop::construct_from_paths(image_path, camera_path, boxes[0], datum, alt_range));
  EXPECT_EQ(empty_image.cols(), 0);
  EXPECT_EQ(empty_image.rows(), 0);

  OrbitalImageCrop entire_image(OrbitalImageCrop::construct_from_paths(image_path, camera_path, boxes[3], datum, alt_range));
  EXPECT_EQ(entire_image.cols(), image_nocrop.cols());
  EXPECT_EQ(entire_image.rows(), image_nocrop.rows());
}


TEST(OrbtialImageCropCollection, add_image) {
  // Set up test space
  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);

  {
    // Outside footprint
    OrbitalImageCropCollection collect(BBox2(55.9392, 9.21995, 0.1, 0.1), datum, alt_range);
    collect.add_image(SrcName("synth.0.tif"), SrcName("synth.0.pinhole"));
    EXPECT_EQ(collect.size(), 0u);
  }

  {
    // Entire footprint
    OrbitalImageCropCollection collect(BBox2(55, 9, 2, 1), datum, alt_range);
    collect.add_image(SrcName("synth.0.tif"), SrcName("synth.0.pinhole"));
    EXPECT_EQ(collect.size(), 1u);
  }
}

void no_crop_helper(OrbitalImageCropCollection &collect, vector<Vector2i> &image_sizes, 
                    std::string const& image_path, std::string const& camera_path) {
  boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
  image_sizes.push_back(Vector2i(rsrc->cols(), rsrc->rows()));

  collect.add_image(image_path, camera_path);
}

TEST(OrbitalImageCropCollection, no_crop) {
  vector<Vector2i> image_sizes;

  OrbitalImageCropCollection collect;

  no_crop_helper(collect, image_sizes, SrcName("synth.0.tif"), SrcName("synth.0.pinhole"));
  no_crop_helper(collect, image_sizes, SrcName("synth.1.tif"), SrcName("synth.1.pinhole"));
  no_crop_helper(collect, image_sizes, SrcName("synth.2.tif"), SrcName("synth.2.pinhole"));
  no_crop_helper(collect, image_sizes, SrcName("synth.3.tif"), SrcName("synth.3.pinhole"));
  EXPECT_EQ(collect.size(), 4u);

  for (unsigned i = 0; i < collect.size(); i++) {
    EXPECT_VECTOR_EQ(image_sizes[i], Vector2i(collect[i].cols(), collect[i].rows()));
  }
}

#if MVP_ENABLE_OCTAVE_SUPPORT
TEST(OrbitalImageCropCollection, to_octave) {
  // Set up test space
  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);

  OrbitalImageCropCollection collect(BBox2(55, 9, 2, 1), datum, alt_range); // Should catch all footprints
  collect.add_image(SrcName("synth.0.tif"), SrcName("synth.0.pinhole"));
  collect.add_image(SrcName("synth.1.tif"), SrcName("synth.1.pinhole"));
  collect.add_image(SrcName("synth.2.tif"), SrcName("synth.2.pinhole"));
  collect.add_image(SrcName("synth.3.tif"), SrcName("synth.3.pinhole"));
  EXPECT_EQ(collect.size(), 4u);

  ::octave_map oct_collect(collect.to_octave());
  EXPECT_EQ(oct_collect.dims()(0), 1);
  EXPECT_EQ(oct_collect.dims()(1), 4);
  EXPECT_EQ(oct_collect.keys()(0), "data");
  EXPECT_EQ(oct_collect.keys()(1), "camera");
}
#endif
