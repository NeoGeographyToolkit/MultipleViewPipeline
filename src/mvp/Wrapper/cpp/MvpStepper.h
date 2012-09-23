/// \file MvpStepper.h
///
/// TODO: Write doc
///

#ifndef __MVP_OCTAVE_MVPSTEPPER_H__
#define __MVP_OCTAVE_MVPSTEPPER_H__

#include <mvp/Wrapper/MvpWrapper.h>

#include <mvp/Algorithm/Stepper/StupidStepper.h>

MVP_WRAPPER_BEGINC(MvpStepper, mvp::algorithm::Stepper) {

  MVP_WRAPPER_CONSTRUCTOR("StupidStepper") {
    MVP_WRAPPER_CONSTRUCTOR_RETURN(mvp::algorithm::StupidStepper());
  }

} MVP_WRAPPER_ENDC()

MVP_WRAPPER_BEGINF(mvp::algorithm::Stepper) {

  MVP_WRAPPER_FUNCTION("zing") {
    MVP_WRAPPER_FUNCTION_NARGS(2);
    MVP_WRAPPER_FUNCTION_ARG(int, arg1);
    MVP_WRAPPER_FUNCTION_ARG(int, arg2);
    MVP_WRAPPER_FUNCTION_VOID(obj->zing(arg1, arg2));
  }

  MVP_WRAPPER_FUNCTION("zap") {
    MVP_WRAPPER_FUNCTION_NARGS(1);
    MVP_WRAPPER_FUNCTION_ARG(int, arg1);
    MVP_WRAPPER_FUNCTION_RETURN(obj->zap(arg1));
  }

} MVP_WRAPPER_ENDF()

#endif
