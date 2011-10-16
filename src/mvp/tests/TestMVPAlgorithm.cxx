#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/MVPAlgorithm.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace mvp;

TEST(MVPAlgorithm, NullImpl) {
  MVPAlgorithmSettings settings;

  settings.set_test_algorithm(true);

  MVPAlgorithm algo(settings, OrbitalImageCropCollection());

  MVPAlgorithmResult result = algo(MVPAlgorithmVar(), GeoReference());

  EXPECT_EQ(result.post_height, 0);
  EXPECT_VECTOR_EQ(result.orientation, Vector3f());
  EXPECT_VECTOR_EQ(result.windows, Vector3f());

  EXPECT_EQ(result.variance, 0);
  EXPECT_TRUE(result.converged);
  EXPECT_EQ(result.num_iterations_to_converge, 0);
}
