#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/MVPTileSeeder.h>
#include <mvp/MVPAlgorithm.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace mvp;

// TODO: Write proper tests here

struct MVPAlgoTest : public MVPAlgorithm {
  virtual MVPPixelResult operator()(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const {
    return MVPPixelResult(seed);
  }
};

TEST(MVPTileSeederDumbTest, init) {
  MVPAlgoTest algorithm;
  MVPTileSeederDumb seeder(&algorithm, GeoReference(Datum("D_MOON")), 256, MVPUserSettings());

  EXPECT_TRUE(seeder.init());
}

TEST(MVPTileSeederSquareTest, seed) {
  MVPAlgoTest algorithm;
  MVPTileSeederSquare seeder(&algorithm, GeoReference(Datum("D_MOON")), 256, MVPUserSettings());

  seeder.update(0, 0, MVPAlgorithmVar(3));
  seeder.update(0, 1, MVPAlgorithmVar(4));
  seeder.update(0, 2, MVPAlgorithmVar(5));

  EXPECT_NEAR(seeder.seed(0, 0).alt, 4, 1e-3);
}
