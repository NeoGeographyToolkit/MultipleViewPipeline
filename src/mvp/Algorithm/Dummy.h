#ifndef __MVP_ALGORITHM_DUMMY_H__
#define __MVP_ALGORITHM_DUMMY_H__

#include <mvp/Algorithm/ObjectBase.h>

#include <mvp/Algorithm/DummySettings.pb.h>

#include <vw/Math/Vector.h>

#define ALGORITHM_OBJECT_Dummy(T) \
BEGIN_##T(Dummy, mvp::algorithm::Dummy, (int)(int)(mvp::algorithm::DummySettings const&)) \
  T(void0, (void)) \
  T(void1, (void)(int)) \
  T(void2, (void)(int)(int)) \
  T(function0, (int)) \
  T(function1, (int)(int)) \
  T(function2, (int)(int)(int)) \
  T##_C(x, (int)) \
  T##_C(y, (int)) \
  T(do_vector, (vw::Vector2)(vw::Vector3 const&)) \
  T(do_vector, (vw::Vector2)(vw::Vector2 const&)) \
END_##T()

EMIT_ALGORITHM_OBJECT(Dummy)

#endif
