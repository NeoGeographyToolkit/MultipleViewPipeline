#ifndef __MVP_ALGORITHM_CORRELATOR_H__
#define __MVP_ALGORITHM_CORRELATOR_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/PixelResult.h>
#include <mvp/Algorithm/Lighter.h>
#include <mvp/Algorithm/Objective.h>
#include <mvp/Image/OrbitalImageCollection.h>

#include <mvp/Algorithm/CorrelatorSettings.pb.h>

#define ALGORITHM_OBJECT_Correlator(T) \
BEGIN_##T(Correlator, mvp::algorithm::Correlator, (mvp::image::OrbitalImageCollection const&) \
          (mvp::algorithm::Lighter const&)(mvp::algorithm::Objective const&) \
          (mvp::algorithm::CorrelatorSettings const&)) \
  T##_C(correlate, (mvp::algorithm::PixelResult)(vw::Vector3 const&)(mvp::algorithm::AlgorithmVar const&)) \
END_##T()

EMIT_ALGORITHM_OBJECT(Correlator)

#endif
