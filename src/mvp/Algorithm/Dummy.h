#ifndef __MVP_ALGORITHM_DUMMY_H__
#define __MVP_ALGORITHM_DUMMY_H__

#include <mvp/Algorithm/AlgoBase.h>

#include <vw/Math/Vector.h>

namespace mvp {
namespace algorithm {

struct Dummy : public AlgoBase<Dummy, int, int> {
  protected:
    Dummy();
  public:
    Dummy(std::string const& type, bool use_octave, int x, int y) : 
      AlgoBase<Dummy, int, int>(type, use_octave, x, y) {}

  virtual void void0() {return impl()->void0();}

  virtual void void1(int a) {return impl()->void1(a);}

  virtual void void2(int a, int b) {return impl()->void2(a,b);}

  virtual int function0() {return impl()->function0();}

  virtual int function1(int a) {return impl()->function1(a);}

  virtual int function2(int a, int b) {return impl()->function2(a,b);}

  virtual int x() const {return impl()->x();}

  virtual int y() const {return impl()->y();}

  virtual vw::Vector2 do_vector(vw::Vector3 const& a) {return impl()->do_vector(a); }

  virtual vw::Vector2 do_vector(vw::Vector2 const& a) {return impl()->do_vector(a); }

};

}} // namespace algorithm,mvp

#endif
