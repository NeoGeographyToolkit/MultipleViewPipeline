#ifndef __MVP_ALGORITHM_SEEDER_H__
#define __MVP_ALGORITHM_SEEDER_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/PixelResult.h>
#include <mvp/Algorithm/Seed.h>

#include <vw/Cartography/GeoReference.h>

#include <mvp/Algorithm/SeederSettings.pb.h>

#define ALGORITHM_OBJECT_Seeder(T) \
BEGIN_##T(Seeder, mvp::algorithm::Seeder, (vw::cartography::GeoReference const&)(vw::Vector2i) \
          (mvp::algorithm::SeederSettings const&)) \
  T##_C(curr_post, (vw::Vector3)) \
  T##_C(curr_seed, (mvp::algorithm::Seed)) \
  T##_C(done, (bool)) \
  T##_C(result, (std::vector<mvp::algorithm::Seed>)) \
  T(update, (void)(mvp::algorithm::PixelResult)) \
END_##T()

EMIT_ALGORITHM_OBJECT(Seeder)

#endif
