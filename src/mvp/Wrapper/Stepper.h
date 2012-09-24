/// \file OctaveStepper.h
///
/// TODO: Write doc
///

#ifndef __MVP_WRAPPER_STEPPER_H__
#define __MVP_WRAPPER_STEPPER_H__

#include <mvp/Wrapper/OctaveWrapper.h>
#include <mvp/Wrapper/MvpWrapper.h>

#include <mvp/Algorithm/Stepper/StupidStepper.h>

namespace mvp {
namespace wrapper {

/// OctaveWrapper

OCT_WRAPPER_BEGIN(OctaveStepper, mvp::algorithm::Stepper) {
  OCT_WRAPPER_INIT();

  OCT_WRAPPER_VOID2(zing, int, int);

  OCT_WRAPPER_FUNCTION1(int, zap, int);

} OCT_WRAPPER_END()

/// MvpWrapper

MVP_WRAPPER_BEGINC(MvpStepper, mvp::algorithm::Stepper) {

  MVP_WRAPPER_CONSTRUCTOR("StupidStepper") {
    MVP_WRAPPER_CONSTRUCTOR_RETURN(mvp::algorithm::StupidStepper());
  }

} MVP_WRAPPER_ENDC()

MVP_WRAPPER_BEGIN(mvp::algorithm::Stepper) {

  MVP_WRAPPER_VOID2(zing, int, int);

  MVP_WRAPPER_FUNCTION1(int, zap, int);

} MVP_WRAPPER_END()


}} // namespace wrapper,mvp

#endif
