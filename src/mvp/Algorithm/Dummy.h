#ifndef __MVP_ALGORITHM_DUMMY_H__
#define __MVP_ALGORITHM_DUMMY_H__

#include <vw/Math/Vector.h>
#include <boost/shared_ptr.hpp>

namespace mvp {
namespace algorithm {

struct DummyBase {
  virtual ~DummyBase() {}

  virtual void void0() = 0;

  virtual void void1(int a) = 0;

  virtual void void2(int a, int b) = 0;

  virtual int function0() = 0;

  virtual int function1(int a) = 0;

  virtual int function2(int a, int b) = 0;

  virtual int x() = 0;

  virtual int y() = 0;

  virtual vw::Vector2 do_vector(vw::Vector3 const& a) = 0;
};

class Dummy {
  boost::shared_ptr<DummyBase> m_impl;

  public:
    Dummy(std::string const& type);

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
