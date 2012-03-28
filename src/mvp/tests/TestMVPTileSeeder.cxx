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
    return MVPPixelResult();
  }
};

TEST(MVPTileSeederDumbTest, init) {
  MVPAlgoTest algorithm;
  MVPTileSeederDumb seeder(&algorithm, GeoReference(Datum("D_MOON")), 256, MVPUserSettings());

  EXPECT_TRUE(seeder.init());
}
