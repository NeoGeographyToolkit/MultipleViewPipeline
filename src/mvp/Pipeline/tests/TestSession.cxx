#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Pipeline/Session.h>
#include <mvp/Core/Settings.h>

using namespace std;
using namespace mvp;
using namespace vw::test;
using namespace mvp::core;
using namespace mvp::pipeline;

TEST(Session, session_desc) {
  SessionDesc session_desc;
  load_settings(&session_desc, SrcName("test.mvp"));

  session_desc.mutable_input()->set_image_pattern(DataName("synth\\.\\d*\\.tif"));
  session_desc.mutable_input()->set_camera_pattern(DataName("synth\\.\\d*\\.pinhole"));

  Session session(session_desc);

  EXPECT_TRUE(session.has_next_job());
  JobDesc job_desc = session.next_job();

  EXPECT_EQ(job_desc.render().col(), 1);
  EXPECT_EQ(job_desc.render().row(), 2);

  while (session.has_next_job()) {
    job_desc = session.next_job();
  }

  EXPECT_EQ(job_desc.render().col(), 2);
  EXPECT_EQ(job_desc.render().row(), 3);
}
