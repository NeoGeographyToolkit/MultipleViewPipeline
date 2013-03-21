#ifndef __MVP_ALGORITHM_DUMMY_H__
#define __MVP_ALGORITHM_DUMMY_H__

#include <mvp/Algorithm/ObjectBase.h>

#include <mvp/Algorithm/DummySettings.pb.h>

#include <vw/Math/Vector.h>

BEGIN_ALGORITHM_OBJECT(Dummy, mvp::algorithm::Dummy, (int)(int)(DummySettings const&))
  ALGORITHM_OBJECT(void0, (void))
  ALGORITHM_OBJECT(void1, (void)(int))
  ALGORITHM_OBJECT(void2, (void)(int)(int))
  ALGORITHM_OBJECT(function0, (int))
  ALGORITHM_OBJECT(function1, (int)(int))
  ALGORITHM_OBJECT(function2, (int)(int)(int))
  ALGORITHM_OBJECT_C(x, (int))
  ALGORITHM_OBJECT_C(y, (int))
  ALGORITHM_OBJECT(do_vector, (vw::Vector2)(vw::Vector3 const&))
  ALGORITHM_OBJECT(do_vector, (vw::Vector2)(vw::Vector2 const&))
END_ALGORITHM_OBJECT()

#endif
