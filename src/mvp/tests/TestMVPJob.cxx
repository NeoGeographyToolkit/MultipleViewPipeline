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

struct MVPJobSeedTest : public MVPJobBase<MVPJobSeedTest> {
  MVPJobSeedTest(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPAlgorithmSettings const& settings) :
    MVPJobBase<MVPJobSeedTest>(georef, tile_size, crops, settings) {}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const {
    static int num_calls = 0;
    // int col = georef.transform()(0, 2) - m_georef.transform()(0, 2);
    // int row = georef.transform()(1, 2) - m_georef.transform()(1, 2);
    return MVPPixelResult(seed, ++num_calls);
  }
};

TEST(MVPJob, seeding) {
  MVPJobSeedTest job(GeoReference(), 256, OrbitalImageCropCollection(), MVPAlgorithmSettings());
  MVPTileResult result(job.process_tile());
}

TEST(Helpers, offset_georef) {
  GeoReference geo = PlateGeoReference().tile_georef(1, 2, 3);
  GeoReference crop_geo = offset_georef(geo, 100, 200);

  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(100, 200)), crop_geo.pixel_to_lonlat(Vector2(0, 0)));
  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(120, 230)), crop_geo.pixel_to_lonlat(Vector2(20, 30)));
}

MVPJobRequest create_job_request(bool use_octave = true) {
  // Set up workspace
  const int tile_size = 64;
  const Datum datum("D_MOON");
  const Vector2 alt_limits(0, 0);

  PlateGeoReference plate_georef(datum, "equi", tile_size, GeoReference::PixelAsPoint);
  MVPAlgorithmSettings settings;
  settings.set_test_algorithm(true);
  settings.set_use_octave(use_octave);
  settings.set_alt_min(alt_limits[0]);
  settings.set_alt_max(alt_limits[1]);

  MVPWorkspace work("", "", plate_georef, settings);
  work.add_image_pattern(SrcName("synth.%d.tif"), SrcName("synth.%d.pinhole"), 0, 3);
  EXPECT_EQ(work.num_images(), 4);

  // Determine work levels, create the job request
  int level = work.equal_resolution_level();
  Vector2 col_row = work.tile_work_area(level).min();

  return work.assemble_job(col_row[0], col_row[1], level);
}

TEST(MVPJob, save_job_file) {
  // TODO: Finish test here
  MVPJobRequest job_request(create_job_request());
  string job_file = save_job_file(job_request);
}

void process_tile_test(MVPJobRequest job_request) {
  // Don't verify all pixels in result, only verify every fourth one for speed
  const int validation_divisor = 4;

  // Process the tile 
  MVPTileResult result(mvpjob_process_tile(job_request));

  PlateGeoReference plate_georef(job_request.plate_georef());
  Vector2 col_row = Vector2(job_request.col(), job_request.row());
  int level = job_request.level();

  // Set up variables for the reference calculation
  OrbitalImageCropCollection orbital_images;
  orbital_images.add_image_collection(job_request.orbital_images());

  GeoReference georef(plate_georef.tile_georef(col_row[0], col_row[1], level));
  EXPECT_EQ(georef.build_desc().DebugString(), result.georef.build_desc().DebugString());

  int min_overlap = numeric_limits<int>::max();
  int max_overlap = 0;

  // Manually calculate overlaps, and compare to the result from MVPJob
  for (int i = 0; i < result.alt.cols(); i += validation_divisor) {
    for (int j = 0; j < result.alt.rows(); j += validation_divisor) {
      Vector2 ll = georef.pixel_to_lonlat(Vector2(i, j));
      Vector3 llr(ll[0], ll[1], georef.datum().radius(ll[0], ll[1]));
      Vector3 xyz = lon_lat_radius_to_xyz(llr);

      int overlaps = 0;
      BOOST_FOREACH(OrbitalImageCrop o, orbital_images) {
        Vector2 px = o.camera().point_to_pixel(xyz);
        if (bounding_box(o).contains(px)) {
          overlaps++;
        }
      }

      PixelMask<float32> px_result(overlaps);
      PixelMask<Vector3f> px_result3(overlaps, overlaps, overlaps);
      if (!overlaps) {
        px_result.invalidate();
        px_result3.invalidate();
      }

      EXPECT_TYPE_EQ(px_result, result.alt(i, j));
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

TEST(MVPJob, process_tile) {
  process_tile_test(create_job_request(false));
}

#if MVP_ENABLE_OCTAVE_SUPPORT
TEST(MVPJob, process_tile_octave) {
  process_tile_test(create_job_request(true));
}

TEST(MVPAlgorithmVar, to_octave) {
  MVPAlgorithmVar var(1, Vector3(2, 3, 4), Vector3(5, 6, 7));

  ::octave_scalar_map oct_var(var.to_octave());

  MVPAlgorithmVar var2(oct_var);

  EXPECT_EQ(var.alt, var2.alt);
  EXPECT_VECTOR_NEAR(var.orientation, var2.orientation, 1e-6);
  EXPECT_VECTOR_NEAR(var.windows, var2.windows, 1e-6);
}

TEST(MVPPixelResult, octave_construct) {
  MVPAlgorithmVar var(1, Vector3(2, 3, 4), Vector3(5, 6, 7));

  ::octave_value_list oct_result;
  oct_result.append(var.to_octave());
  oct_result.append(8);
  oct_result.append(true);
  oct_result.append(9);

  MVPPixelResult result(oct_result);

  EXPECT_EQ(result.alt, var.alt);
  EXPECT_VECTOR_NEAR(result.orientation, var.orientation, 1e-6);
  EXPECT_VECTOR_NEAR(result.windows, var.windows, 1e-6);
  EXPECT_EQ(result.variance, 8);
  EXPECT_TRUE(result.converged);
  EXPECT_EQ(result.num_iterations, 9);
}
#endif
