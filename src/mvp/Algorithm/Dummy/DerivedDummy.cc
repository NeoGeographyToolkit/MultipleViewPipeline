#include <mvp/Algorithm/Dummy/DerivedDummy.h>

namespace mvp {
namespace algorithm {

REGISTER_ALGORITHM_IMPL(Dummy, DerivedDummy);

DerivedDummy::DerivedDummy(int x, int y) : m_x(x), m_y(y) {}

void DerivedDummy::void0() {}

void DerivedDummy::void1(int a) {}

void DerivedDummy::void2(int a, int b) {}

int DerivedDummy::function0() { return 0; }

int DerivedDummy::function1(int a) { return a; }

int DerivedDummy::function2(int a, int b) { return a + b; }

int DerivedDummy::x() { return m_x; }

int DerivedDummy::y() { return m_y; }

vw::Vector2 DerivedDummy::do_vector(vw::Vector3 const& a) { return vw::math::subvector(a, 0, 2); }

}} // namespace algorithm,mvp
