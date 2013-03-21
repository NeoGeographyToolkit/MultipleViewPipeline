#include <mvp/Algorithm/Dummy.h>

namespace mvp {
namespace algorithm {

struct DerivedDummy : public Dummy {
  int m_x;
  int m_y;

  DerivedDummy(int x, int y) : m_x(x), m_y(y) {}

  void void0() {}

  void void1(int a) {}

  void void2(int a, int b) {}

  int function0() { return 0; }

  int function1(int a) { return a; }

  int function2(int a, int b) { return a + b; }

  int x() const { return m_x; }

  int y() const { return m_y; }

  vw::Vector2 do_vector(vw::Vector3 const& a) { return vw::math::subvector(a, 0, 2); }

  vw::Vector2 do_vector(vw::Vector2 const& a) { return a; }
};

REGISTER_ALGORITHM_OBJECT(DerivedDummy);

}} // namespace algorithm,mvp
