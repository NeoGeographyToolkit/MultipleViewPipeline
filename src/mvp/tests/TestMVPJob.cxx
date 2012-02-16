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
  ImageView<float32> m_dem;

  MVPJobSeedTest() :
    MVPJobBase<MVPJobSeedTest>(GeoReference(Datum("D_MOON")), 256, OrbitalImageCropCollection(), MVPUserSettings()),
    m_dem(m_tile_size, m_tile_size) 
  {
    m_settings.set_alt_min(0);
    m_settings.set_alt_max(0);
    m_settings.set_alt_search_range(1);
    m_settings.set_seed_window_size(20);
    m_settings.set_seed_window_smooth_size(4);

    fill(crop(m_dem, 145, 81, 32, 32), 1);
    fill(crop(m_dem, 209, 145, 32, 16), 1);
  }

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, double col, double row, MVPAlgorithmOptions const& options) const {
    return MVPJobBase::process_pixel(seed, col, row, options);
  }

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    // Multiply window size (in sigma) by 6 to get window size in pixels
    Vector2 pixelWinSize = Vector2(seed.windows.x(), seed.windows.y()) * 6;
    
    BBox2 patchWin(0, 0, pixelWinSize.x(), pixelWinSize.y());
    patchWin -= (pixelWinSize - Vector2(1, 1)) / 2;

    Vector2 patchCenter = Vector2(georef.transform()(0, 2), georef.transform()(1, 2)) -
                          Vector2(m_georef.transform()(0, 2), m_georef.transform()(1, 2));

    BBox2 demWinPre = patchWin + patchCenter;
  
    BBox2i demWin(round(demWinPre.min().x()), round(demWinPre.min().y()),
                  round(demWinPre.width()), round(demWinPre.height()));

    // Calculate the sum of pixel values of m_dem inside the patch window.
    float32 sum = sum_of_pixel_values(crop(m_dem, demWin));

    return MVPPixelResult(MVPAlgorithmVar(sum), 0, sum > 0);
  }
};

TEST(MVPJob, MVPJobSeedTest) {
  MVPJobSeedTest job;

  MVPAlgorithmVar seed(0, Vector3f(), Vector3f(32, 32, 0) / 6);

  EXPECT_EQ(job.process_pixel(seed, 145, 81, MVPAlgorithmOptions()).alt, 16*16);
  EXPECT_EQ(job.process_pixel(seed, 177, 81, MVPAlgorithmOptions()).alt, 16*16);
  EXPECT_EQ(job.process_pixel(seed, 145, 113, MVPAlgorithmOptions()).alt, 16*16);
  EXPECT_EQ(job.process_pixel(seed, 177, 113, MVPAlgorithmOptions()).alt, 16*16);

  EXPECT_EQ(job.process_pixel(seed, 209, 145, MVPAlgorithmOptions()).alt, 16*16);
  EXPECT_EQ(job.process_pixel(seed, 241, 145, MVPAlgorithmOptions()).alt, 16*16);

  seed.windows = Vector3f(64, 64, 0) / 6;
  
  EXPECT_EQ(job.process_pixel(seed, 161, 97, MVPAlgorithmOptions()).alt, 32*32);
  EXPECT_EQ(job.process_pixel(seed, 161, 33, MVPAlgorithmOptions()).alt, 0);

}

TEST(MVPJob, seeding) {
  MVPJobSeedTest job;
  //MVPTileResult result(job.process_tile());
  //job.generate_seed();
}

TEST(Helpers, crop_georef) {
  // Make sure the crop function in VW is working the way
  // we expect it to
  GeoReference geo = PlateGeoReference().tile_georef(1, 2, 3);
  GeoReference crop_geo = crop(geo, 100.5, 200.5);

  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(100.5, 200.5)), crop_geo.pixel_to_lonlat(Vector2(0, 0)));
  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(120.5, 230.5)), crop_geo.pixel_to_lonlat(Vector2(20, 30)));
}

MVPJobRequest create_job_request(bool use_octave = false) {
  // Set up workspace
  const int tile_size = 64;
  const Datum datum("D_MOON");

  PlateGeoReference plate_georef(datum, "equi", tile_size, GeoReference::PixelAsPoint);
  MVPUserSettings settings;
  settings.set_alt_min(0);
  settings.set_alt_max(0);
  settings.set_alt_search_range(-1);
  settings.set_seed_window_size(-1);
  settings.set_seed_window_smooth_size(-1);
  settings.set_test_algorithm(true);
  settings.set_use_octave(use_octave);

  MVPWorkspace work("", "", plate_georef, settings);
  work.add_image_pattern(SrcName("synth.%d.tif"), SrcName("synth.%d.pinhole"), 0, 3);
  EXPECT_EQ(work.num_images(), 4);

  // Determine work levels, create the job request
  int level = work.equal_resolution_level();
  Vector2 col_row = work.tile_work_area(level).min() + Vector2(1, 0);
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
