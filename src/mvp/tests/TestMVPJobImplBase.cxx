#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/MVPJobImplBase.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace vw::camera;
using namespace mvp;

struct MVPJobImplSeedTest : public MVPJobImplBase {
  ImageView<float32> m_dem;

  MVPJobImplSeedTest() : 
    MVPJobImplBase(GeoReference(Datum("D_MOON")), 256, OrbitalImageCropCollection(), MVPUserSettings()),
    m_dem(m_tile_size, m_tile_size) 
  {
    fill(crop(m_dem, 144, 80, 32, 32), 1);
    fill(crop(m_dem, 208, 144, 32, 16), 1);
  }

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    // Multiply window size (in sigma) by the gauss_divisor to get window size in pixels
    Vector2 pixelWinSize = Vector2(seed.windows.x(), seed.windows.y()) * options.gauss_divisor();
    
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

TEST(MVPJobSeedTest, process_pixel) {
  // TODO: write me
}

TEST(MVPJobSeedTest, process_tile) {
  // TODO: write me
}

TEST(Helpers, crop_georef) {
  // Make sure the crop function in VW is working the way
  // we expect it to
  GeoReference geo;
  GeoReference crop_geo = crop(geo, 100.5, 200.5);

  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(100.5, 200.5)), crop_geo.pixel_to_lonlat(Vector2(0, 0)));
  EXPECT_VECTOR_EQ(geo.pixel_to_lonlat(Vector2(120.5, 230.5)), crop_geo.pixel_to_lonlat(Vector2(20, 30)));
}

#if MVP_ENABLE_OCTAVE_SUPPORT
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
