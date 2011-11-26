/// \file Main.h
///
/// TODO: Write doc
///

#ifndef __VW_OCTAVE_MAIN_H__
#define __VW_OCTAVE_MAIN_H__

#include <octave/oct.h>

#include <octave/parse.h>
#include <octave/octave.h>
#include <octave/toplev.h>

namespace vw {
namespace octave {

void start_octave_interpreter(std::string const& startup_script = "") {
  const char * argvv [] = {"", "--silent"};
  ::octave_main (2, (char **) argvv, true);
  if (!startup_script.empty()) {
    ::source_file(startup_script);
  }
}

}} // namespace vw, octave

#endif
