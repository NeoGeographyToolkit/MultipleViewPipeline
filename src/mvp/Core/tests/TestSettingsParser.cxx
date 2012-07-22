#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Core/SettingsParser.h>
#include <mvp/Core/tests/TestProto.pb.h>

using namespace vw::test;

TEST(SettingsParser, parse_settings) {
  mvp::core::TestSettings settings;
  mvp::core::parse_settings(&settings, SrcName("TestProto.ini"));
  std::cout << settings.DebugString() << std::endl;
}

