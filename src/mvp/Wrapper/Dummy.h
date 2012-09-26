/// \file Dummy.h
///
/// TODO: Write doc
///

#ifndef __MVP_WRAPPER_DUMMY_H__
#define __MVP_WRAPPER_DUMMY_H__

#include <mvp/Wrapper/OctaveWrapper.h>
#include <mvp/Wrapper/MvpWrapper.h>

#include <mvp/Algorithm/Dummy/DerivedDummy.h>

namespace mvp {
namespace wrapper {

OCT_WRAPPER_BEGIN(OctaveDummy, mvp::algorithm::Dummy) {
  OCT_WRAPPER_INIT();

  OCT_WRAPPER_CONSTRUCT();

  OCT_WRAPPER_CONSTRUCT1(int);
  
  OCT_WRAPPER_CONSTRUCT2(int, int);

  OCT_WRAPPER_VOID(void0);

  OCT_WRAPPER_VOID1(void1, int);

  OCT_WRAPPER_VOID2(void2, int, int);

  OCT_WRAPPER_FUNCTION(int, function0);

  OCT_WRAPPER_FUNCTION1(int, function1, int);

  OCT_WRAPPER_FUNCTION2(int, function2, int, int);
  
  OCT_WRAPPER_FUNCTION(int, x);
  
  OCT_WRAPPER_FUNCTION(int, y);

} OCT_WRAPPER_END

}} // namespace wrapper, mvp

MVP_WRAPPER_BEGIN(MvpDummy, mvp::algorithm::Dummy) {

  MVP_WRAPPER_CONSTRUCT(mvp::algorithm::DerivedDummy, "DerivedDummy");

  MVP_WRAPPER_CONSTRUCT1(mvp::algorithm::DerivedDummy, "DerivedDummy", int);

  MVP_WRAPPER_CONSTRUCT2(mvp::algorithm::DerivedDummy, "DerivedDummy", int, int);

  MVP_WRAPPER_VOID(void0);

  MVP_WRAPPER_VOID1(void1, int);

  MVP_WRAPPER_VOID2(void2, int, int);

  MVP_WRAPPER_FUNCTION(int, function0);

  MVP_WRAPPER_FUNCTION1(int, function1, int);

  MVP_WRAPPER_FUNCTION2(int, function2, int, int);
  
  MVP_WRAPPER_FUNCTION(int, x);
  
  MVP_WRAPPER_FUNCTION(int, y);

} MVP_WRAPPER_END

#endif
