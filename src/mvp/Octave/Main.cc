#include <mvp/Octave/Main.h>

#include <mvp/Core/Settings.h>

#include <mvp/Octave/MvpWrapper.h>

#include <octave/parse.h>
#include <octave/octave.h>
#include <octave/toplev.h>

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

namespace {
  octave_value_list do_mvp_settings(octave_value_list const& args, int nargout) {
    if (args.length() > 0) {
      mvp_settings() = from_octave<mvp::core::GlobalSettings>(args(0));
    }
    return to_octave(mvp_settings());
  }
}

void register_octave_mvp() {
  octave_mvpclass_ref::register_type();
  MvpWrapperInstaller::install_wrappers();
  install_builtin_function(do_mvp_settings, "mvp_settings", "");
}

void stop_octave_interpreter() {
  do_octave_atexit();
}

}} // namespace mvp, octave
