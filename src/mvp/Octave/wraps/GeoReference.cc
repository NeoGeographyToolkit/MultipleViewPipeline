#include <mvp/Octave/MvpWrapper.h>
#include <vw/Cartography/GeoReference.h>

BEGIN_MVP_WRAPPER(GeoReference, vw::cartography::GeoReference, MVP_WRAPPER_NO_ARGS)
  MVP_WRAPPER_CONSTRUCTOR((vw::cartography::Datum const&))
  MVP_WRAPPER_CONSTRUCTOR((vw::cartography::Datum const&)(vw::Matrix3x3 const&))
  MVP_WRAPPER_CONSTRUCTOR((vw::cartography::Datum const&)(vw::Matrix3x3 const&)
                          (vw::cartography::GeoReference::PixelInterpretation))
  MVP_WRAPPER_C(lonlat_to_pixel, (vw::Vector2)(vw::Vector2))
  MVP_WRAPPER_C(pixel_to_lonlat, (vw::Vector2)(vw::Vector2))
  MVP_WRAPPER_C(datum, (vw::cartography::Datum const&))
END_MVP_WRAPPER()
