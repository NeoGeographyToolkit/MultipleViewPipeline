#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Image/ImageCrop.h>

#include <vw/Image/Algorithms.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

#include <boost/foreach.hpp>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;
using namespace mvp::image;

TEST(HelperFunction, crop_pinhole) {
  PinholeModel cam(SrcName("data/synth.0.pinhole"));
  PinholeModel crop_cam(crop(cam, BBox2i(10, 20, 100, 100)));
 
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(10, 20)), crop_cam.pixel_to_vector(Vector2(0, 0)), 1e-6);
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(20, 30)), crop_cam.pixel_to_vector(Vector2(10, 10)), 1e-6);
}

TEST(ImageCrop, find_crop_bbox) {
  // TODO: write me
}

TEST(ImageCrop, construct_from_paths) {
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
  string image_path(SrcName("data/synth.0.tif"));
  string camera_path(SrcName("data/synth.0.pinhole"));
  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);
  Vector2 radius_range(alt_range + Vector2(datum.semi_major_axis(), datum.semi_major_axis()));

  // Reference image, not cropped
  ImageCrop image_nocrop(ImageCrop::construct_from_paths(image_path, camera_path));

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
    ImageCrop image_crop(ImageCrop::construct_from_paths(image_path, camera_path, b, datum, alt_range));

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

  ImageCrop empty_image(ImageCrop::construct_from_paths(image_path, camera_path, boxes[0], datum, alt_range));
  EXPECT_EQ(empty_image.cols(), 0);
  EXPECT_EQ(empty_image.rows(), 0);

  ImageCrop entire_image(ImageCrop::construct_from_paths(image_path, camera_path, boxes[3], datum, alt_range));
  EXPECT_EQ(entire_image.cols(), image_nocrop.cols());
  EXPECT_EQ(entire_image.rows(), image_nocrop.rows());
}
