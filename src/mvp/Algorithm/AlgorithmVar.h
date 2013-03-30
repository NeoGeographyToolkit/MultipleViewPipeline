#ifndef __MVP_ALGORITHM_ALGORITHMVAR_H__
#define __MVP_ALGORITHM_ALGORITHMVAR_H__

#include <mvp/Algorithm/ObjectBase.h>

#include <mvp/Core/GlobalSettings.pb.h>

#include <vw/Math/Vector.h>
#include <vw/Math/Quaternion.h>

namespace mvp {
namespace algorithm {
typedef vw::Vector<double, 12> AlgorithmVarVector;
}}

#define ALGORITHM_OBJECT_AlgorithmVar(T) \
BEGIN_##T(AlgorithmVar, mvp::algorithm::AlgorithmVar, \
          (mvp::algorithm::AlgorithmVarVector const&)(mvp::core::GlobalSettings::AlgorithmVarSettings const&)) \
  T##_C(radius, (double)) \
  T##_C(orientation, (vw::Quat)) \
  T##_C(window, (vw::Vector2)) \
  T##_C(gwindow, (vw::Vector2)) \
  T##_C(smooth, (double)) \
  T##_C(gsmooth, (double)) \
  T##_C(scale, (double)) \
  T##_C(vectorize, (mvp::algorithm::AlgorithmVarVector)) \
END_##T()

EMIT_ALGORITHM_OBJECT(AlgorithmVar)

#endif
