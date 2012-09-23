/// \file MvpStepper.h
///
/// TODO: Write doc
///

#ifndef __MVP_OCTAVE_MVPSTEPPER_H__
#define __MVP_OCTAVE_MVPSTEPPER_H__

#include <mvp/Wrapper/MvpWrapper.h>

#include <mvp/Algorithm/Stepper/StupidStepper.h>

OCT_WRAP_BEGINC(MvpStepper, mvp::algorithm::Stepper) {

  OCT_WRAP_CONSTRUCTOR("StupidStepper") {
    OCT_WRAP_CONSTRUCTOR_RETURN(mvp::algorithm::StupidStepper());
  }

} OCT_WRAP_ENDC()

OCT_WRAP_BEGINF(mvp::algorithm::Stepper) {

  OCT_WRAP_FUNCTION("zing") {
    OCT_WRAP_FUNCTION_NARGS(2);
    OCT_WRAP_FUNCTION_ARG(int, arg1);
    OCT_WRAP_FUNCTION_ARG(int, arg2);
    OCT_WRAP_FUNCTION_VOID(obj->zing(arg1, arg2));
  }

  OCT_WRAP_FUNCTION("zap") {
    OCT_WRAP_FUNCTION_NARGS(1);
    OCT_WRAP_FUNCTION_ARG(int, arg1);
    OCT_WRAP_FUNCTION_RETURN(obj->zap(arg1));
  }

} OCT_WRAP_ENDF()

#endif
