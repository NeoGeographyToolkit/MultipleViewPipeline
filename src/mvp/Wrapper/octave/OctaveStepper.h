/// \file OctaveStepper.h
///
/// TODO: Write doc
///

#ifndef __MVP_WRAPPER_OCTAVESTEPPER_H__
#define __MVP_WRAPPER_OCTAVESTEPPER_H__

#include <mvp/Wrapper/OctaveWrapper.h>

#include <mvp/Algorithm/Stepper.h>

namespace mvp {
namespace wrapper {

OCT_WRAPPER_BEGIN(OctaveStepper, mvp::algorithm::Stepper) {
  OCT_WRAPPER_INIT();

  OCT_WRAPPER_VOID2(zing, int, int);

  OCT_WRAPPER_FUNCTION1(int, zap, int);

} OCT_WRAPPER_END()

}} // namespace wrapper,mvp

#endif
