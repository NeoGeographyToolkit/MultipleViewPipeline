#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/MVPJob.h>
#include <mvp/MVPWorkspace.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace vw::camera;
using namespace mvp;

TEST(MVPJob, process_tile) {
  // Don't verify all pixels in result, only verify every fourth one for speed
  const int validation_divisor = 4;

  // Set up workspace
  const int tile_size = 64;
  const Datum datum("D_MOON");
  const Vector2 post_height_limits(0, 0);

  PlateGeoReference plate_georef(datum, "equi", tile_size, GeoReference::PixelAsPoint);
  MVPAlgorithmSettings settings;
  settings.set_test_algorithm(true);
  settings.set_post_height_limit_min(post_height_limits[0]);
  settings.set_post_height_limit_max(post_height_limits[1]);

  MVPWorkspace work("", "", plate_georef, settings);
  work.add_image_pattern(SrcName("synth.%d.tif"), SrcName("synth.%d.pinhole"), 0, 3);
  EXPECT_EQ(work.num_images(), 4);

  // Determine work levels, create the job request
  int level = work.equal_resolution_level();
  Vector2 col_row = work.tile_work_area(level).min();

  MVPJobRequest job_request = work.assemble_job(col_row[0], col_row[1], level);
 
  // Process the tile 
  MVPTileResult result = mvpjob_process_tile(job_request);

  // Set up variables for the reference calculation
  vector<ImageView<PixelMask<PixelGray<float32> > > > orbital_images;
  vector<PinholeModel> cameras;

  BOOST_FOREACH(OrbitalImageFileDescriptor d, job_request.orbital_images()) {
    orbital_images.push_back(create_mask(DiskImageView<PixelGray<float32> >(d.image_path()), numeric_limits<float32>::quiet_NaN()));
    cameras.push_back(PinholeModel(d.camera_path()));
  }

  GeoReference georef(plate_georef.tile_georef(col_row[0], col_row[1], level));
  EXPECT_EQ(georef.build_desc().DebugString(), result.georef.build_desc().DebugString());

  int min_overlap = numeric_limits<int>::max();
  int max_overlap = 0;

  // Manually calculate overlaps, and compare to the result from MVPJob
  for (int i = 0; i < result.post_height.cols(); i += validation_divisor) {
    for (int j = 0; j < result.post_height.rows(); j += validation_divisor) {
      Vector2 ll = georef.pixel_to_lonlat(Vector2(i, j));
      Vector3 llr(ll[0], ll[1], georef.datum().radius(ll[0], ll[1]));
      Vector3 xyz = lon_lat_radius_to_xyz(llr);

      int overlaps = 0;
      for (unsigned k = 0; k < orbital_images.size(); k++) {
        Vector2 px = cameras[k].point_to_pixel(xyz);
        if (bounding_box(orbital_images[k]).contains(px)) {
          overlaps++;
        }
      }

      PixelMask<float32> px_result(overlaps);
      PixelMask<Vector3f> px_result3(overlaps, overlaps, overlaps);
      if (!overlaps) {
        px_result.invalidate();
        px_result3.invalidate();
      }

      EXPECT_TYPE_EQ(px_result, result.post_height(i, j));
      EXPECT_TYPE_EQ(px_result, result.variance(i, j));
      EXPECT_TYPE_EQ(px_result3, result.orientation(i, j));
      EXPECT_TYPE_EQ(px_result3, result.windows(i, j));

      min_overlap = min(overlaps, min_overlap);
      max_overlap = max(overlaps, max_overlap);
    }
  }

  // Make sure we saw both no images AND all images
  EXPECT_EQ(min_overlap, 0);
  EXPECT_EQ(max_overlap, 4);
}

TEST(Helpers, offset_georef) {
  GeoReference geo = PlateGeoReference().tile_georef(1, 2, 3);
  GeoReference crop_geo = offset_georef(geo, 100, 200);

  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(100, 200)), crop_geo.pixel_to_lonlat(Vector2(0, 0)));
  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(120, 230)), crop_geo.pixel_to_lonlat(Vector2(20, 30)));
}
