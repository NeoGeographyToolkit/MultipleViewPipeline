#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Core/Settings.h>

using namespace std;
using namespace mvp;
using namespace vw::test;

TEST(Settings, mvp_settings) {
  mvp_settings(SrcName("mvprc"));

  EXPECT_EQ(mvp_settings().ports().command(), 23);
  EXPECT_EQ(mvp_settings().ports().status(), 6678);
}
