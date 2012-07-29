#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Frontend/Session.h>

using namespace std;
using namespace mvp;
using namespace vw::test;
using namespace mvp::core;
using namespace mvp::pipeline;
using namespace mvp::frontend;

TEST(Settings, mvp_settings) {
  SessionDesc session_desc;
  load_settings(&session_desc, SrcName("test.mvp"));

  Session session(session_desc);

  EXPECT_TRUE(session.has_next());
  JobDesc job_desc = session.next();

  EXPECT_EQ(job_desc.render().col(), 1);
  EXPECT_EQ(job_desc.render().row(), 2);

  while (session.has_next()) {
    job_desc = session.next();
  }

  EXPECT_EQ(job_desc.render().col(), 2);
  EXPECT_EQ(job_desc.render().row(), 3);
}
