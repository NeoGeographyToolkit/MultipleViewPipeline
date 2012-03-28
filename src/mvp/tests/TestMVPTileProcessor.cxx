#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/MVPTileProcessor.h>
#include <mvp/MVPTileSeeder.h>
#include <mvp/MVPAlgorithm.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace mvp;

// TODO: Write better tests

struct MVPAlgoTest : public MVPAlgorithm {
  virtual MVPPixelResult operator()(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    return MVPPixelResult();
  }
};

struct MVPTileSeederTest : public MVPTileSeeder {
  int m_num_counts;

  MVPTileSeederTest(MVPAlgorithm *algorithm, vw::cartography::GeoReference georef, int tile_size, MVPUserSettings user_settings) :
    MVPTileSeeder(algorithm, georef, tile_size, user_settings) {}

  bool init() {
    m_num_counts = 0;
    return true;
  }

  MVPAlgorithmVar seed(int col, int row) const {
    return MVPAlgorithmVar();
  }

  MVPPixelResult update(int col, int row, MVPAlgorithmVar const& seed) {
    MVPPixelResult px_result(MVPAlgorithmVar(m_num_counts++));
    m_result.update(col, row, px_result);
    
    return px_result;
  }
};

TEST(MVPTileProcessorDumb, operator) {
  MVPAlgoTest algorithm;
  MVPTileSeederTest seeder(&algorithm, GeoReference(Datum("D_MOON")), 256, MVPUserSettings());
  MVPTileProcessorDumb processor(&seeder);

  MVPTileResult result(processor());

  int num = 0;
  for (int i = 0; i < result.tile_size; i++) {
    for (int j = 0; j < result.tile_size; j++) {
      EXPECT_EQ(result.alt(i, j), num++);
    }
  }  
}
