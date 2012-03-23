#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/MVPJob.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace vw::camera;
using namespace mvp;

/*

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

    fill(crop(m_dem, 144, 80, 32, 32), 1);
    fill(crop(m_dem, 208, 144, 32, 16), 1);
  }

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, double col, double row, MVPAlgorithmOptions const& options) const {
    return MVPJobBase<MVPJobSeedTest>::process_pixel(seed, col, row, options);
  }

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    // Multiply window size (in sigma) by GAUSS_DIVISOR to get window size in pixels
    Vector2 pixelWinSize = Vector2(seed.windows.x(), seed.windows.y()) * GAUSS_DIVISOR;
    
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

  list<MVPSeedBBox> expected_seeds() {
    list<MVPSeedBBox> result;

    result.push_back(MVPSeedBBox(MVPAlgorithmVar(16*16), BBox2i(192, 128, 32, 32), 1));
    result.push_back(MVPSeedBBox(MVPAlgorithmVar(16*16), BBox2i(224, 128, 32, 32), 1));

    result.push_back(MVPSeedBBox(MVPAlgorithmVar(16*16), BBox2i(128, 64, 32, 32), 1));
    result.push_back(MVPSeedBBox(MVPAlgorithmVar(16*16), BBox2i(160, 96, 32, 32), 1));
    result.push_back(MVPSeedBBox(MVPAlgorithmVar(16*16), BBox2i(128, 96, 32, 32), 1));
    result.push_back(MVPSeedBBox(MVPAlgorithmVar(16*16), BBox2i(160, 64, 32, 32), 1));

    return result;
  }
};

TEST(MVPJobSeedTest, process_pixel) {
  MVPJobSeedTest job;

  list<MVPSeedBBox> expected_seeds(job.expected_seeds());

  BOOST_FOREACH(MVPSeedBBox const& es, expected_seeds) {
    MVPAlgorithmVar seed(0, Vector3f(), Vector3f(32, 32, 0) / GAUSS_DIVISOR);

    Vector2 center_pt = (es.bbox.min() + es.bbox.max() - Vector2(1, 1)) / 2;

    EXPECT_EQ(job.process_pixel(seed, center_pt.x(), center_pt.y(), MVPAlgorithmOptions()).alt, es.seed.alt);
  }

  MVPAlgorithmVar seed(0, Vector3f(), Vector3f(64, 64, 0) / GAUSS_DIVISOR);
  
  EXPECT_EQ(job.process_pixel(seed, 160, 96, MVPAlgorithmOptions()).alt, 32*32);
  EXPECT_EQ(job.process_pixel(seed, 160, 32, MVPAlgorithmOptions()).alt, 0);
}

TEST(MVPJob, generate_seeds) {
  MVPJobSeedTest job;

  list<MVPSeedBBox> seeds(job.generate_seeds());
  list<MVPSeedBBox> expected_seeds(job.expected_seeds());

  // Assert here so that if they're not equal the for loop doesn't run
  ASSERT_EQ(seeds.size(), expected_seeds.size());

  list<MVPSeedBBox>::const_iterator iter1, iter2;

  for (iter1 = seeds.begin(), iter2 = expected_seeds.begin();
       iter1 != seeds.end(); iter1++, iter2++) {
    EXPECT_EQ(iter1->seed.alt, iter2->seed.alt);
    EXPECT_VECTOR_EQ(iter1->bbox.min(), iter2->bbox.min());
    EXPECT_VECTOR_EQ(iter1->bbox.max(), iter2->bbox.max());
    EXPECT_EQ(iter1->alt_range, iter2->alt_range);
  }
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

  PlateGeoReference plate_geo(datum, "equi", tile_size, GeoReference::PixelAsPoint);
  MVPUserSettings user_settings;
  user_settings.set_alt_min(0);
  user_settings.set_alt_max(0);
  user_settings.set_alt_search_range(-1);
  user_settings.set_seed_window_size(3);
  user_settings.set_seed_window_smooth_size(-1);
  user_settings.set_test_algorithm(true);
  user_settings.set_use_octave(use_octave);

  MVPWorkspaceRequest work_request;
  work_request.set_result_platefile("result");
  work_request.set_internal_result_platefile("internal");
  *work_request.mutable_plate_georef() = plate_geo.build_desc();
  *work_request.mutable_user_settings() = user_settings;

  for (int i = 0; i <= 3; i++) {
    stringstream ss;
    ss << "synth." << i;
    OrbitalImageFileDescriptor image;
    image.set_image_path(SrcName(ss.str() + ".tif"));
    image.set_camera_path(SrcName(ss.str() + ".pinhole"));
    *work_request.add_orbital_images() = image;
  }

  MVPWorkspace work(work_request);

  // Determine work levels, create the job request
  int level = work.render_level();
  Vector2 col_row = work.render_bbox().min() + Vector2(3, 3);
  return work.assemble_job(col_row[0], col_row[1], level);
}

TEST(MVPJob, save_job_file) {
  // TODO: Finish test here
  MVPJobRequest job_request(create_job_request());
  string job_file = save_job_file(job_request);
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

*/
