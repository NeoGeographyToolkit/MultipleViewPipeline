#ifndef __MVP_ALGORITHM_STEPPER_H__
#define __MVP_ALGORITHM_STEPPER_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/Seed.h>
#include <mvp/Algorithm/TileResult.h>

#include <mvp/Algorithm/StepperSettings.pb.h>

#define ALGORITHM_OBJECT_Stepper(T) \
BEGIN_##T(Stepper, mvp::algorithm::Stepper, (vw::cartography::GeoReference const&)(vw::Vector2i) \
          (mvp::algorithm::StepperSettings const&)) \
  T##_C(curr_pixel, (vw::Vector2i)) \
  T##_C(curr_post, (vw::Vector3)) \
  T##_C(curr_seed, (mvp::algorithm::Seed)) \
  T##_C(done, (bool)) \
  T##_C(result, (mvp::algorithm::TileResult)) \
  T(update, (void)(mvp::algorithm::PixelResult const&)) \
END_##T()

EMIT_ALGORITHM_OBJECT(Stepper)

#endif
