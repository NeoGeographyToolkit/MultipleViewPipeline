/// \file MvpStepper.h
///
/// TODO: Write doc
///

#ifndef __MVP_OCTAVE_MVPSTEPPER_H__
#define __MVP_OCTAVE_MVPSTEPPER_H__

#include <mvp/Wrapper/MvpWrapper.h>

#include <mvp/Algorithm/Stepper/StupidStepper.h>

namespace mvp {
namespace wrapper {

MVP_WRAPPER_BEGINC(MvpStepper, mvp::algorithm::Stepper) {

  MVP_WRAPPER_CONSTRUCTOR("StupidStepper") {
    MVP_WRAPPER_CONSTRUCTOR_RETURN(mvp::algorithm::StupidStepper());
  }

} MVP_WRAPPER_ENDC()

MVP_WRAPPER_BEGIN(mvp::algorithm::Stepper) {

  MVP_WRAPPER_VOID2(zing, int, int);

  MVP_WRAPPER_FUNCTION1(int, zap, int);

} MVP_WRAPPER_END()

}} //namespace wrapper, mvp

#endif
