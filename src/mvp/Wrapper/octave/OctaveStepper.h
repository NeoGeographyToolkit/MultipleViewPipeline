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

  void zing(int a, int b) {
    OCT_WRAPPER_FUNCTION_INIT();
    OCT_WRAPPER_FUNCTION_ARG(a);
    OCT_WRAPPER_FUNCTION_ARG(b);
    OCT_WRAPPER_FUNCTION_VOID("zing");
  }

  int zap(int a) {
    OCT_WRAPPER_FUNCTION_INIT(); 
    OCT_WRAPPER_FUNCTION_ARG(a);
    return OCT_WRAPPER_FUNCTION(int, "zap");
  }

} OCT_WRAPPER_END()

}} // namespace wrapper,mvp

#endif
