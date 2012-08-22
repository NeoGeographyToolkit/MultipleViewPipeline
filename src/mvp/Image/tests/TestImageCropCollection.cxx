#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Image/ImageCropCollection.h>

#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/FileIO/DiskImageResource.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::camera;
using namespace vw::cartography;
using namespace mvp::image;

TEST(ImageCropCollection, add_image) {
  // Set up test space
  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);

  {
    // Outside footprint
    ImageCropCollection collect(BBox2(55.9392, 9.21995, 0.1, 0.1), datum, alt_range);
    collect.add_image(DataName("synth.0.tif"), DataName("synth.0.pinhole"));
    EXPECT_EQ(collect.size(), 0u);
  }

  {
    // Entire footprint
    ImageCropCollection collect(BBox2(55, 9, 2, 1), datum, alt_range);
    collect.add_image(DataName("synth.0.tif"), DataName("synth.0.pinhole"));
    EXPECT_EQ(collect.size(), 1u);
  }
}

void no_crop_helper(ImageCropCollection &collect, vector<Vector2i> &image_sizes, 
                    std::string const& image_path, std::string const& camera_path) {
  boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
  image_sizes.push_back(Vector2i(rsrc->cols(), rsrc->rows()));

  collect.add_image(image_path, camera_path);
}

TEST(ImageCropCollection, no_crop) {
  vector<Vector2i> image_sizes;

  ImageCropCollection collect;

  no_crop_helper(collect, image_sizes, DataName("synth.0.tif"), DataName("synth.0.pinhole"));
  no_crop_helper(collect, image_sizes, DataName("synth.1.tif"), DataName("synth.1.pinhole"));
  no_crop_helper(collect, image_sizes, DataName("synth.2.tif"), DataName("synth.2.pinhole"));
  no_crop_helper(collect, image_sizes, DataName("synth.3.tif"), DataName("synth.3.pinhole"));
  EXPECT_EQ(collect.size(), 4u);

  for (unsigned i = 0; i < collect.size(); i++) {
    EXPECT_VECTOR_EQ(image_sizes[i], Vector2i(collect[i].cols(), collect[i].rows()));
  }
}

#if MVP_ENABLE_OCTAVE_SUPPORT
TEST(ImageCropCollection, to_octave) {
  // Set up test space
  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);

  ImageCropCollection collect(BBox2(55, 9, 2, 1), datum, alt_range); // Should catch all footprints
  collect.add_image(DataName("synth.0.tif"), DataName("synth.0.pinhole"));
  collect.add_image(DataName("synth.1.tif"), DataName("synth.1.pinhole"));
  collect.add_image(DataName("synth.2.tif"), DataName("synth.2.pinhole"));
  collect.add_image(DataName("synth.3.tif"), DataName("synth.3.pinhole"));
  EXPECT_EQ(collect.size(), 4u);

  ::octave_map oct_collect(collect.to_octave());
  EXPECT_EQ(oct_collect.dims()(0), 1);
  EXPECT_EQ(oct_collect.dims()(1), 4);
  EXPECT_EQ(oct_collect.keys()(0), "data");
  EXPECT_EQ(oct_collect.keys()(1), "camera");
}
#endif
