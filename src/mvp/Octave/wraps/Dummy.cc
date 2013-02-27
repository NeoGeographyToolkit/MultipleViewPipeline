#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Algorithm/Dummy.h>

namespace mvp {
namespace algorithm {
struct OctaveDummy : public Dummy {
  int m_x;
  int m_y;

  OctaveDummy(std::string const& type, int x, int y) : m_x(x), m_y(y) {}

  void void0() {std::cout << "in octave!" << std::endl;}

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

REGISTER_OCTAVE_ALGORITHMS(OctaveDummy)

}} // namespace algorithm,mvp

BEGIN_MVP_WRAPPER(Dummy, mvp::algorithm::Dummy)
  MVP_WRAP_CONSTRUCTOR((std::string)(bool)(int)(int))
  MVP_WRAP_VOID((void0))
  MVP_WRAP_VOID((void1)(int))
  MVP_WRAP_VOID((void2)(int)(int))
  MVP_WRAP_FUNCTION((function0))
  MVP_WRAP_FUNCTION((function1)(int))
  MVP_WRAP_FUNCTION((function2)(int)(int))
  MVP_WRAP_FUNCTION((do_vector)(vw::Vector3))
END_MVP_WRAPPER()
