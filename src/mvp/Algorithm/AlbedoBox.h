#ifndef __MVP_ALGORITHM_ALBEDOBOX_H__
#define __MVP_ALGORITHM_ALBEDOBOX_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/PatchBox.h>

#include <mvp/Core/GlobalSettings.pb.h>

ALGORITHM_SETTINGS_GLOBAL_DEFAULTER(AlbedoBoxSettings, albedo_box_settings)

#define ALGORITHM_OBJECT_AlbedoBox(T) \
BEGIN_##T(AlbedoBox, mvp::algorithm::AlbedoBox, \
          (mvp::algorithm::PatchBox const&)(vw::Vector<double> const&)(vw::Vector<double> const&) \
          (mvp::core::GlobalSettings::AlbedoBoxSettings const&)) \
  T##_C(albedo, (vw::ImageView<double>)) \
  T##_C(weight, (vw::ImageView<double>)) \
  T##_C(global_albedo, (vw::ImageView<double>)) \
  T##_C(a, (vw::Vector<double>)) \
  T##_C(b, (vw::Vector<double>)) \
  T##_C(weight_sum, (vw::ImageView<double>)) \
  T##_C(weight_sum2, (vw::ImageView<double>)) \
  T##_C(rows, (int)) \
  T##_C(cols, (int)) \
  T##_C(planes, (int)) \
END_##T()

EMIT_ALGORITHM_OBJECT(AlbedoBox)

#endif
