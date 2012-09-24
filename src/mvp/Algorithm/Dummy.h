#ifndef __MVP_ALGORITHM_DUMMY_H__
#define __MVP_ALGORITHM_DUMMY_H__

namespace mvp {
namespace algorithm {

struct Dummy {
  virtual ~Dummy() {}

  virtual void void0() = 0;

  virtual void void1(int a) = 0;

  virtual void void2(int a, int b) = 0;

  virtual int function0() = 0;

  virtual int function1(int a) = 0;

  virtual int function2(int a, int b) = 0;

  virtual int x() = 0;

  virtual int y() = 0;
};

}} // namespace algorithm,mvp

#endif
