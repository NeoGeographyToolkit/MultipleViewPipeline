#ifndef __MVP_ALGORITHM_PATCHBOX_H__
#define __MVP_ALGORITHM_PATCHBOX_H__

#include <mvp/Algorithm/ObjectBase.h>

#include <mvp/Core/GlobalSettings.pb.h>

#include <mvp/Image/OrbitalImage.h>

ALGORITHM_SETTINGS_GLOBAL_DEFAULTER(PatchBoxSettings, patch_box_settings)

#define ALGORITHM_OBJECT_PatchBox(T) \
BEGIN_##T(PatchBox, mvp::algorithm::PatchBox, \
          (std::vector<vw::ImageView<mvp::image::OrbitalImagePixel> > const&)(vw::Vector2 const&)(double)(double) \
          (mvp::core::GlobalSettings::PatchBoxSettings const&)) \
  T##_C(window, (vw::ImageView<double>)) \
  T##_C(intensity, (vw::ImageView<double>)) \
  T##_C(intensity2, (vw::ImageView<double>)) \
  T##_C(weight, (vw::ImageView<double>)) \
  T##_C(weight_sum, (vw::ImageView<double>)) \
  T##_C(weight_sum2, (vw::ImageView<double>)) \
  T##_C(rows, (int)) \
  T##_C(cols, (int)) \
  T##_C(planes, (int)) \
END_##T()

EMIT_ALGORITHM_OBJECT(PatchBox)

#endif
