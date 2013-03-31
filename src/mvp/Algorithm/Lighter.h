#ifndef __MVP_ALGORITHM_LIGHTER_H__
#define __MVP_ALGORITHM_LIGHTER_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/PatchBox.h>
#include <mvp/Algorithm/AlbedoBox.h>

#include <mvp/Algorithm/LighterSettings.pb.h>

#define ALGORITHM_OBJECT_Lighter(T) \
BEGIN_##T(Lighter, mvp::algorithm::Lighter, (mvp::algorithm::LighterSettings const&)) \
  T##_C(light, (mvp::algorithm::AlbedoBox)(mvp::algorithm::PatchBox const&)) \
END_##T()

EMIT_ALGORITHM_OBJECT(Lighter)

#endif
