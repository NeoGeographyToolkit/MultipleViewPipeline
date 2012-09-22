/// \file Stepper.h
///
/// Defines mvp Stepper interface
///
/// TODO: Write something here
///

#ifndef __MVP_ALGORITHM_STEPPER_H__
#define __MVP_ALGORITHM_STEPPER_H__

namespace mvp {
namespace algorithm {

class Stepper {

  public:
    virtual ~Stepper() {}
    virtual void zing(int, int) = 0;
    virtual int zap(int) = 0;
};

}}

#endif
