#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/MVPJobImpl.h>

#include <vw/Plate/PlateGeoReference.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace vw::camera;
using namespace mvp;

template <class JobT>
class MVPJobImplFootprintTest : public ::testing::Test {
  protected:
    virtual void SetUp() {
      // tile_size
      tile_size = 64;
 
      // helpers
      int col = 5374;
      int row = 3875;
      int level = 13;
      PlateGeoReference plate_georef(Datum("D_MOON"), "equi", tile_size, GeoReference::PixelAsPoint);
    
      // georef 
      georef = plate_georef.tile_georef(col, row, level);

      // orbital_images
      for (int i = 0; i <= 3; i++) {
        stringstream ss;
        ss << "synth." << i;
        OrbitalImageFileDescriptor image;
        image.set_image_path(SrcName(ss.str() + ".tif"));
        image.set_camera_path(SrcName(ss.str() + ".pinhole"));
        orbital_images.push_back(image);
      }

      // JobT
      OrbitalImageCropCollection crops(plate_georef.tile_lonlat_bbox(col, row, level),
                                       plate_georef.datum(), 0, 0);
      crops.add_image_collection(orbital_images);
      impl.reset(new JobT(georef, tile_size, crops, MVPUserSettings()));
    }

    int tile_size;
    GeoReference georef;
    vector<OrbitalImageFileDescriptor> orbital_images;
    boost::shared_ptr<MVPJobImplBase> impl;
};

#if MVP_ENABLE_OCTAVE_SUPPORT
typedef ::testing::Types<MVPJobImplFootprint, MVPJobImplFootprintOctave> FootprintTypes;
#else
typedef ::testing::Types<MVPJobImplFootprint> FootprintTypes;
#endif
TYPED_TEST_CASE(MVPJobImplFootprintTest, FootprintTypes);

TYPED_TEST(MVPJobImplFootprintTest, process_pixel) {
  // Don't verify all pixels in result, only verify every second one for speed
  const int validation_divisor = 2;

  // Use uncropped images for the reference calculation
  OrbitalImageCropCollection crops;
  crops.add_image_collection(this->orbital_images);

  // Store the results here
  MVPTileResult result(this->georef, this->tile_size);
  MVPTileResult result_ref(this->georef, this->tile_size);

  // Keep track of the min/max overlap seen
  int min_overlap = numeric_limits<int>::max();
  int max_overlap = 0;

  // Manually calculate overlaps, and compare to the result from MVPJobImplFootprint
  for (int i = 0; i < this->tile_size; i += validation_divisor) {
    for (int j = 0; j < this->tile_size; j += validation_divisor) {
      Vector2 ll = this->georef.pixel_to_lonlat(Vector2(i, j));
      // TODO: this calculation only works for spheroid datums
      Vector3 llr(ll[0], ll[1], this->georef.datum().radius(ll[0], ll[1]));
      Vector3 xyz = lon_lat_radius_to_xyz(llr);

      int overlaps = 0;
      BOOST_FOREACH(OrbitalImageCrop o, crops) {
        Vector2 px = o.camera().point_to_pixel(xyz);
        if (bounding_box(o).contains(px)) {
          overlaps++;
        }
      }

      MVPAlgorithmVar px_var(overlaps, Vector3f(overlaps, overlaps, overlaps), Vector3f(overlaps, overlaps, overlaps));
      MVPPixelResult px_result(px_var, overlaps, overlaps > 0, overlaps);
      result_ref.update(i, j, px_result);      

      result.update(i, j, this->impl->process_pixel(MVPAlgorithmVar(), i, j, MVPAlgorithmOptions()));

      min_overlap = min(overlaps, min_overlap);
      max_overlap = max(overlaps, max_overlap);
    }
  }

  // Make this test covers something interesting
  EXPECT_EQ(min_overlap, 0);
  EXPECT_GT(max_overlap, 1);

  // Verify against reference calculation
  for (int i = 0; i < this->tile_size; i += validation_divisor) {
    for (int j = 0; j < this->tile_size; j += validation_divisor) {
      EXPECT_TYPE_EQ(result.alt(i, j), result_ref.alt(i, j));
      EXPECT_TYPE_EQ(result.variance(i, j), result_ref.variance(i, j));
      EXPECT_TYPE_EQ(result.orientation(i, j), result_ref.orientation(i, j));
      EXPECT_TYPE_EQ(result.windows(i, j), result_ref.windows(i, j));
    }
  }

  // For debug
  //write_image("alt.tif", result.alt);
}
