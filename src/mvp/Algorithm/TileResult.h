#ifndef __MVP_ALGORITHM_TILERESULT_H__
#define __MVP_ALGORITHM_TILERESULT_H__

#include <mvp/Algorithm/ObjectBase.h>
#include <mvp/Algorithm/PixelResult.h>

#include <vw/Cartography/GeoReference.h>
#include <vw/Image/ImageView.h>

#include <mvp/Core/GlobalSettings.pb.h>

ALGORITHM_SETTINGS_GLOBAL_DEFAULTER(TileResultSettings, tile_result_settings)

#define ALGORITHM_OBJECT_TileResult(T) \
BEGIN_##T(TileResult, mvp::algorithm::TileResult, (vw::cartography::GeoReference const&)(vw::Vector2i const&) \
          (mvp::core::GlobalSettings::TileResultSettings const&)) \
  T(set, (void)(vw::Vector2i const&)(mvp::algorithm::PixelResult const&)) \
  T##_C(get, (mvp::algorithm::PixelResult)(vw::Vector2i const&)) \
  T##_C(plate_layer, (vw::ImageView<double>)(double)) \
  T##_C(alt, (vw::ImageView<double>)) \
  T##_C(converged, (vw::ImageView<bool>)) \
END_##T()

EMIT_ALGORITHM_OBJECT(TileResult)

#endif
