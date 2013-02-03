#ifndef __MVP_ALGORITHM_DUMMY_H__
#define __MVP_ALGORITHM_DUMMY_H__

#include <vw/Math/Vector.h>
#include <boost/shared_ptr.hpp>

namespace mvp {
namespace algorithm {

class Dummy {
  boost::shared_ptr<Dummy> m_impl;

  public:
    Dummy(std::string const& type);

    virtual void void0();

    virtual void void1(int a);

    virtual void void2(int a, int b);

    virtual int function0();

    virtual int function1(int a);

    virtual int function2(int a, int b);

    virtual int x();

    virtual int y();

    virtual vw::Vector2 do_vector(vw::Vector3 const& a);

  protected:
    // Only subclasses can construct without a impl
    Dummy() {}
};

}} // namespace algorithm,mvp

#endif
