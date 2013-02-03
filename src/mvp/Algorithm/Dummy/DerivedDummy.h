#ifndef __MVP_ALGORITHM_DERIVEDDUMMY_H__
#define __MVP_ALGORITHM_DERIVEDDUMMY_H__

#include <mvp/Algorithm/Dummy.h>

namespace mvp {
namespace algorithm {

struct DerivedDummy : public Dummy {
  int m_x;
  int m_y;

  DerivedDummy();

  DerivedDummy(int x);

  DerivedDummy(int x, int y);

  void void0();

  void void1(int a);

  void void2(int a, int b);

  int function0();

  int function1(int a);

  int function2(int a, int b);

  int x();

  int y();

  vw::Vector2 do_vector(vw::Vector3 const& a);
};

}} // namespace algorithm,mvp

#endif
