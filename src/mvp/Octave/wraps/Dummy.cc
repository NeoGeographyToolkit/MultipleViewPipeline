#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Algorithm/Dummy.h>
#include <mvp/Octave/OctaveWrapper.h>

namespace mvp {
namespace octave {

struct OctaveDummy : public mvp::algorithm::Dummy {
  mutable OctaveWrapperImpl m_octave_impl;

  OctaveDummy(std::string const& type, int x, int y) : m_octave_impl(type, x, y) {}

  OctaveDummy(octave_value const& impl) : m_octave_impl(impl) {}

  void void0() { m_octave_impl.wrap_function("void0"); }

  void void1(int a) { m_octave_impl.wrap_function("void1", a); }

  void void2(int a, int b) { m_octave_impl.wrap_function("void2", a, b); }

  int function0() { return octave_as<int>(m_octave_impl.wrap_function("function0")); }

  int function1(int a) { return octave_as<int>(m_octave_impl.wrap_function("function1", a)); }

  int function2(int a, int b) { return octave_as<int>(m_octave_impl.wrap_function("function2", a, b)); }

  int x() const { return octave_as<int>(m_octave_impl.wrap_function("x")); }

  int y() const { return octave_as<int>(m_octave_impl.wrap_function("y")); }

  vw::Vector2 do_vector(vw::Vector3 const& a) { return octave_as<vw::Vector2>(m_octave_impl.wrap_function("do_vector")); }
  
  vw::Vector2 do_vector(vw::Vector2 const& a) { return octave_as<vw::Vector2>(m_octave_impl.wrap_function("do_vector")); }
};
}} // namespace octave,mvp
template <>
mvp::algorithm::Dummy octave_wrapper<mvp::algorithm::Dummy>(octave_value const& impl) {
  return mvp::octave::OctaveDummy(impl);
}
namespace mvp {
namespace algorithm {
REGISTER_OCTAVE_ALGORITHMS(octave::OctaveDummy)
}}
 

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
