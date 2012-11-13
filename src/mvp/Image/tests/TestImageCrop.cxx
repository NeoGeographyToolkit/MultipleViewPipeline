#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Image/ImageCrop.h>

#include <vw/Image/Statistics.h>
#include <vw/Image/MaskViews.h>
#include <vw/Image/Algorithms.h>
#include <vw/Image/UtilityViews.h>
#include <vw/Image/Filter.h>
#include <vw/FileIO/DiskImageResource.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

#include <boost/foreach.hpp>
#include <boost/random/linear_congruential.hpp>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;
using namespace mvp::image;

TEST(HelperFunction, crop_pinhole) {
  PinholeModel cam(DataName("synth.0.pinhole"));
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
  string image_path(DataName("synth.0.tif"));
  string camera_path(DataName("synth.0.pinhole"));
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

ImageCrop synth_crop(Vector3 const& camera_center, Vector3 const& axis_angle, 
                     Vector2 const& focus, Vector2i size)
{
  boost::rand48 gen;
  PinholeModel camera(camera_center, axis_angle_to_matrix(axis_angle), 
                      focus.x(), focus.y(), size.x() / 2.0, size.y() / 2.0);

  ImageCrop image(gaussian_filter(uniform_noise_view(gen, size.x(), size.y()), 1.5), camera);
  return image; 
}

template <class Image1T, class Image2T>
bool is_image_valid_eq(ImageViewBase<Image1T> const& img1, ImageViewBase<Image2T> const& img2, double tol) {
  double diff = double(sum_of_channel_values(apply_mask(abs(img1.impl() - img2.impl()))));
  int nvalid = sum_of_channel_values(apply_mask(copy_mask(constant_view<int>(1, img1.impl()), img1.impl() - img2.impl())));
  return nvalid && diff < tol; 
}

TEST(ImageCrop, project) {
  Vector2i size(64, 32);

  ImageCrop image;
  ImageData patch;

  // Camera at z = 1, looking back at origin. Plane at z = 0
  image = synth_crop(Vector3(0, 0, 1), Vector3(M_PI, 0, 0), Vector2(1, 1), size);
  patch = image.project(Vector3(0, 0, 0), Quat(1, 0, 0, 0), Vector2(1, 1), size);
  EXPECT_TRUE(is_image_valid_eq(image, patch, 1e-6));

  // Camera at z = 1, looking back at origin. Plane at z = -1
  // Patch is 2f from plane, so to get the original image we make patch with scale 2m/px
  image = synth_crop(Vector3(0, 0, 1), Vector3(M_PI, 0, 0), Vector2(1, 1), size);
  patch = image.project(Vector3(0, 0, -1), Quat(1, 0, 0, 0), Vector2(2, 2), size);
  EXPECT_TRUE(is_image_valid_eq(image, patch, 1e-6));

  // Camera at z = -1, looking toward origin. Plane at z = 0, rotated 180deg about x axis (vertical flip)
  image = synth_crop(Vector3(0, 0, -1), Vector3(0, 0, 0), Vector2(1, 1), size);
  patch = image.project(Vector3(0, 0, 0), Quat(0, 1, 0, 0), Vector2(1, 1), size);
  EXPECT_TRUE(is_image_valid_eq(image, patch, 1e-6));
}
