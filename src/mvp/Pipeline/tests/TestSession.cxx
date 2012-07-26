#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Pipeline/Session.h>
#include <mvp/Core/Settings.h>

using namespace std;
using namespace mvp;
using namespace vw::test;
using namespace mvp::core;
using namespace mvp::pipeline;

TEST(Settings, mvp_settings) {
  SessionDesc session_desc;
  load_settings(&session_desc, SrcName("test.mvp"));

  cout << session_desc.DebugString() << endl;
  //Session session(session_desc);
  
}
