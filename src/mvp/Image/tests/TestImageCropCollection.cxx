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

TEST(ImageCropCollection, push_back) {
  // Set up test space
  Datum datum("D_MOON");
  Vector2 alt_range(-10000, 10000);

  {
    // Outside footprint
    ImageCropCollection collect(BBox2(55.9392, 9.21995, 0.1, 0.1), datum, alt_range);
    collect.push_back(DataName("synth.0.tif"), DataName("synth.0.pinhole"));
    EXPECT_EQ(collect.size(), 0u);
  }

  {
    // Entire footprint
    ImageCropCollection collect(BBox2(55, 9, 2, 1), datum, alt_range);
    collect.push_back(DataName("synth.0.tif"), DataName("synth.0.pinhole"));
    EXPECT_EQ(collect.size(), 1u);
  }
}

void no_crop_helper(ImageCropCollection &collect, vector<Vector2i> &image_sizes, 
                    std::string const& image_path, std::string const& camera_path) {
  boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
  image_sizes.push_back(Vector2i(rsrc->cols(), rsrc->rows()));

  collect.push_back(image_path, camera_path);
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
