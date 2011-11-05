#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/OrbitalImageCrop.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;
using namespace mvp;

TEST(HelperFunction, offset_pinhole) {
  PinholeModel cam(SrcName("synth.0.pinhole"));
  PinholeModel cam_off(offset_pinhole(cam, Vector2(10, 20)));
 
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(10, 20)), cam_off.pixel_to_vector(Vector2(0, 0)), 1e-6);
  EXPECT_VECTOR_NEAR(cam.pixel_to_vector(Vector2(20, 30)), cam_off.pixel_to_vector(Vector2(10, 10)), 1e-6);
}

TEST(OrbtialImageCropCollection, crop) {
/* Footprint for synth.0.pinhole with alt +-10000

    Vector2(56.1474,9.90343)
                                           Vector2(56.6319,9.69012)
                                                                  
                                           
                                           
                                                                      Vector2(56.8948,9.53431)

Vector2(55.9392,9.4308)





                                 Vector2(56.4232,9.21995)
                                                                  Vector2(56.7208,9.14307)
*/
  OrbitalImageFileDescriptor image_file;

  image_file.set_image_path(SrcName("synth.0.tif"));
  image_file.set_camera_path(SrcName("synth.0.pinhole"));

  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);
  
  VW_ASSERT(datum.semi_major_axis() == datum.semi_minor_axis(), LogicErr() << "Spheroid datums not supported");
  Vector2 radius_range(alt_range + Vector2(datum.semi_major_axis(), datum.semi_major_axis()));

  ImageView<PixelMask<PixelGray<float32> > > uncrop_image(create_mask(DiskImageView<PixelGray<float32> >(image_file.image_path()), 
                                                                      numeric_limits<float32>::quiet_NaN()));

  PinholeModel uncrop_camera(image_file.camera_path());

  vector<BBox2> boxes(4);

  boxes[0] = BBox2(55.9392, 9.21995, 0.1, 0.1); // outside footprint
  boxes[1] = BBox2(55.9392, 9.21995, 0.1, 0.3); // overlap footprint
  boxes[2] = BBox2(56.4232, 9.4308, 0.1, 0.1); // inside footprint
  boxes[3] = BBox2(55, 9, 1, 1); // entire footprint

  const int num_pts = 20;
  BOOST_FOREACH(BBox2 const& b, boxes) {
    OrbitalImageCrop image_crop(OrbitalImageCrop::construct_from_descriptor(image_file, b, datum, alt_range));

    for (int i = 0; i < num_pts; i++) {
      for (int j = 0; j < num_pts; j++) {
        for (int k = 0; k < num_pts; k++) {
          double lon = b.width() * i / num_pts + b.min().x();
          double lat = b.height() * j / num_pts + b.min().y();
          double rad = (radius_range[1] - radius_range[0]) * k / num_pts + radius_range[0];
          Vector3 llr(lon, lat, rad);
          Vector3 xyz(lon_lat_radius_to_xyz(llr));

          Vector2 crop_px(image_crop.camera().point_to_pixel(xyz));
          Vector2 uncrop_px(uncrop_camera.point_to_pixel(xyz));

          // If the uncropped image contains the pixel, then the crop MUST have it, and be the same value
          // If the uncropped image doesn't contain the pixel, the crop can have it or not.
          if (bounding_box(uncrop_image).contains(uncrop_px)) {
            bool crop_contains = bounding_box(image_crop.image()).contains(crop_px);
            EXPECT_TRUE(crop_contains);
            if (crop_contains) {
              EXPECT_TYPE_EQ(image_crop.image()(crop_px.x(), crop_px.y()), uncrop_image(uncrop_px.x(), uncrop_px.y()));
            }
          }
        }
      }
    }
  }
}
