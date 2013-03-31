#ifndef __MVP_ALGORITHM_OBJECTIVEHELPER_H__
#define __MVP_ALGORITHM_OBJECTIVEHELPER_H__

#include <mvp/Algorithm/PixelResult.h>
#include <mvp/Algorithm/Lighter.h>
#include <mvp/Algorithm/Objective.h>
#include <mvp/Image/OrbitalImageCollection.h>

#include <mvp/Core/Settings.h>

ALGORITHM_SETTINGS_GLOBAL_DEFAULTER(ObjectiveHelperSettings, objective_helper_settings)

#define ALGORITHM_OBJECT_ObjectiveHelper(T) \
BEGIN_##T(ObjectiveHelper, mvp::algorithm::ObjectiveHelper, (mvp::image::OrbitalImageCollection const&) \
          (mvp::algorithm::Lighter const&)(mvp::algorithm::Objective const&)(vw::Vector3 const&) \
          (mvp::core::GlobalSettings::ObjectiveHelperSettings const&)) \
  T##_C(func, (double)(mvp::algorithm::AlgorithmVar const&)) \
  T##_C(grad, (vw::Matrix<double>)(mvp::algorithm::AlgorithmVar const&)) \
END_##T()

EMIT_ALGORITHM_OBJECT(ObjectiveHelper)

#endif
