#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <mvp/MVPAlgorithm.h>

using namespace std;
using namespace vw;
using namespace vw::test;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace mvp;

TEST(MVPAlgorithm, NullImpl) {
  MVPAlgorithmSettings settings;

  settings.set_null_algorithm(true);

}
