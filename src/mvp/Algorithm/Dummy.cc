#include <mvp/Algorithm/Dummy.h>

#include <mvp/Algorithm/Dummy/DerivedDummy.h>

namespace mvp {
namespace algorithm {

Dummy::Dummy(std::string const& type) {
  if (type == "DerivedDummy") {
    m_impl.reset(new DerivedDummy());
  } else {
    vw::vw_throw(vw::LogicErr() << "Unknown Dummy type");
  }
}

void Dummy::void0() {m_impl->void0();}

void Dummy::void1(int a) {m_impl->void1(a);}

void Dummy::void2(int a, int b) {m_impl->void2(a,b);}

int Dummy::function0() {return m_impl->function0();}

int Dummy::function1(int a) {return m_impl->function1(a);}

int Dummy::function2(int a, int b) {return m_impl->function2(a,b);}

int Dummy::x() {return m_impl->x();}

int Dummy::y() {return m_impl->y();}

vw::Vector2 Dummy::do_vector(vw::Vector3 const& a) { return m_impl->do_vector(a); }

}}
