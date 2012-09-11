/// \file Main.h
///
/// TODO: Write doc
///

#ifndef __VW_OCTAVE_MAIN_H__
#define __VW_OCTAVE_MAIN_H__

#include <string>

namespace vw {
namespace octave {

void start_octave_interpreter(std::string const& startup_script = "");

void stop_octave_interpreter();

}} // namespace vw, octave

#endif
