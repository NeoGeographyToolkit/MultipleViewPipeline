#include <iostream>

#include <mvp/Config.h>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <mvp/Octave/Main.h>
#endif

#include <mvp/Pipeline/Session.h>
#include <mvp/Pipeline/Job.h>
#include <mvp/Core/Settings.h>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>

using namespace std;
using namespace mvp;

int main(int argc, char *argv[]) {

  if (argc != 5) {
    cout << "Usage: mvpdumpjob jobfile col row level" << endl;
    return 1;
  }

  pipeline::SessionDesc session_desc;

  core::load_settings(&session_desc, argv[1]);

  pipeline::Session session(session_desc);

  int col = boost::lexical_cast<int>(argv[2]);
  int row = boost::lexical_cast<int>(argv[3]);
  int level = boost::lexical_cast<int>(argv[4]);

  pipeline::Job job(session.job(col, row, level));

  job.save_job_file();

  #if MVP_ENABLE_OCTAVE_SUPPORT
  octave::start_octave_interpreter(LOADTESTENV_M);
  // TODO: instead
  //octave::start_octave_interpreter();

  job.save_job_file_octave();

  octave::stop_octave_interpreter();
  #endif

  return 0;
}
