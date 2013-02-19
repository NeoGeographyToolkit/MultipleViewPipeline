#include <mvp/Octave/Main.h>

#include <octave/oct.h>
#include <octave/parse.h>
#include <octave/octave.h>
#include <octave/toplev.h>

#include <mvp/Octave/MvpWrapper.h>

namespace mvp {
namespace octave {

void start_octave_interpreter(std::string const& startup_script) {
  const char * argvv [] = {"", "--silent"};
  ::octave_main (2, (char **) argvv, true);
  if (!startup_script.empty()) {
    ::source_file(startup_script);
  }

  register_octave_mvp();
}

void register_octave_mvp() {
  octave_mvpclass_ref::register_type();
  MvpWrapperInstaller::install_wrappers();
}

void stop_octave_interpreter() {
  do_octave_atexit();
}

}} // namespace mvp, octave
