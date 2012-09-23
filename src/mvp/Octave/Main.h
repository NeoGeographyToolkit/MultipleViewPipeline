/// \file Main.h
///
/// TODO: Write doc
///

#ifndef __MVP_OCTAVE_MAIN_H__
#define __MVP_OCTAVE_MAIN_H__

#include <string>

namespace mvp {
namespace octave {

void start_octave_interpreter(std::string const& startup_script = "");

void register_octave_mvp();

void stop_octave_interpreter();

}} // namespace octave, mvp

#endif
