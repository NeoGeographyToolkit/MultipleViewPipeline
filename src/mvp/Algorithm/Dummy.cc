#include <mvp/Algorithm/Dummy.h>

#include <mvp/Algorithm/Dummy/DerivedDummy.h>

namespace mvp {
namespace algorithm {

Dummy::Dummy(std::string const& type) {
  register_algorithm<DerivedDummy>("DerivedDummy");

  set_impl(type);
}

void Dummy::void0() {impl()->void0();}

void Dummy::void1(int a) {impl()->void1(a);}

void Dummy::void2(int a, int b) {impl()->void2(a,b);}

int Dummy::function0() {return impl()->function0();}

int Dummy::function1(int a) {return impl()->function1(a);}

int Dummy::function2(int a, int b) {return impl()->function2(a,b);}

int Dummy::x() {return impl()->x();}

int Dummy::y() {return impl()->y();}

vw::Vector2 Dummy::do_vector(vw::Vector3 const& a) {return impl()->do_vector(a); }

}}
