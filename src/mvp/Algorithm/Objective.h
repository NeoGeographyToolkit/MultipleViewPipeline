#ifndef __MVP_ALGORITHM_OBJECTIVE_H__
#define __MVP_ALGORITHM_OBJECTIVE_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/AlbedoBox.h>

#include <mvp/Algorithm/ObjectiveSettings.pb.h>

#define ALGORITHM_OBJECT_Objective(T) \
BEGIN_##T(Objective, mvp::algorithm::Objective, (mvp::algorithm::ObjectiveSettings const&)) \
  T##_C(f, (double)(mvp::algorithm::AlbedoBox const&)) \
  T##_C(grad, (vw::Matrix<double>)(mvp::algorithm::AlbedoBox const&)) \
END_##T()

EMIT_ALGORITHM_OBJECT(Objective)

#endif
