#ifndef __MVP_ALGORITHM_PIXELRESULT_H__
#define __MVP_ALGORITHM_PIXELRESULT_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/AlgorithmVar.h>

#include <mvp/Core/GlobalSettings.pb.h>

namespace mvp {
namespace algorithm {
typedef vw::Vector<double, 15> PixelResultVector;
}}

#define ALGORITHM_OBJECT_PixelResult(T) \
BEGIN_##T(PixelResult, mvp::algorithm::PixelResult, \
          (mvp::algorithm::AlgorithmVar)(double)(bool)(int)(mvp::core::GlobalSettings::PixelResultSettings const&)) \
  T##_C(algorithm_var, (mvp::algorithm::AlgorithmVar)) \
  T##_C(confidence, (double)) \
  T##_C(converged, (bool)) \
  T##_C(num_iterations, (int)) \
  T##_C(vectorize, (mvp::algorithm::PixelResultVector)) \
END_##T()

EMIT_ALGORITHM_OBJECT(PixelResult)

#endif
