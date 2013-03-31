#ifndef __MVP_ALGORITHM_SEED_H__
#define __MVP_ALGORITHM_SEED_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/PixelResult.h>

#include <mvp/Core/GlobalSettings.pb.h>

ALGORITHM_SETTINGS_GLOBAL_DEFAULTER(SeedSettings, seed_settings)

#define ALGORITHM_OBJECT_Seed(T) \
BEGIN_##T(Seed, mvp::algorithm::Seed, (vw::Vector2 const&)(mvp::algorithm::PixelResult const&) \
          (mvp::core::GlobalSettings::SeedSettings const&)) \
  T##_C(lonlat, (vw::Vector2)) \
  T##_C(value, (mvp::algorithm::PixelResult)) \
END_##T()

EMIT_ALGORITHM_OBJECT(Seed)

#endif
