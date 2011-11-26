// __BEGIN_LICENSE__
// Copyright (C) 2006-2011 United States Government as represented by
// the Administrator of the National Aeronautics and Space Administration.
// All Rights Reserved.
// __END_LICENSE__

#include <gtest/gtest.h>
#include <test/Helpers.h>
#include <vw/Core/Settings.h>
#include <mvp/Config.h>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <vw/Octave/Main.h>
#endif

#include <boost/filesystem/operations.hpp>
namespace fs = boost::filesystem;

int main(int argc, char **argv) {
  #if MVP_ENABLE_OCTAVE_SUPPORT
  vw::octave::start_octave_interpreter(vw::test::BinName("loadtestenv.m"));
  #endif

  // Disable the user's config file
  vw::vw_settings().set_rc_filename("");
  testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();

  #if MVP_ENABLE_OCTAVE_SUPPORT
  do_octave_atexit();
  #endif

  return status;
}

namespace vw {
namespace test {

UnlinkName::UnlinkName(const std::string& base, const std::string& directory)
  : std::string(directory + "/" + base) {

  VW_ASSERT(!directory.empty(), ArgumentErr() << "An empty directory path is dangerous");
  fs::remove_all(this->c_str());
}

UnlinkName::UnlinkName(const char *base, const std::string& directory)
  : std::string(directory + "/" + base) {

  VW_ASSERT(!directory.empty(), ArgumentErr() << "An empty directory path is dangerous");
  fs::remove_all(this->c_str());
}

UnlinkName::~UnlinkName() {
  if (!this->empty())
    fs::remove_all(this->c_str());
}

std::string getenv2(const char *key, const std::string& Default) {
  const char *val = getenv(key);
  return val ? val : Default;
}

}} // namespace vw::test
