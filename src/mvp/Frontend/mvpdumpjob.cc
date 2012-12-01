#include <iostream>

#include <mvp/Pipeline/Session.h>
#include <mvp/Pipeline/Job.h>
#include <mvp/Core/Settings.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace mvp;

int main(int argc, char *argv[]) {

  pipeline::SessionDesc session_desc;

  core::load_settings(&session_desc, argv[1]);

  pipeline::Session session(session_desc);

  int col = boost::lexical_cast<int>(argv[2]);
  int row = boost::lexical_cast<int>(argv[3]);
  int level = boost::lexical_cast<int>(argv[4]);

  pipeline::Job job(session.job(col, row, level));

  job.save_job_file();

  return 0;
}
