#include <mvp/Octave/MvpWrapper.h>
#include <vw/Cartography/GeoReference.h>

BEGIN_MVP_WRAPPER(GeoReference, vw::cartography::GeoReference)
  MVP_WRAP_CONSTRUCTOR((vw::cartography::Datum))
  MVP_WRAP_CONSTRUCTOR((vw::cartography::Datum)(vw::Matrix3x3))
  MVP_WRAP_CONSTRUCTOR((vw::cartography::Datum)(vw::Matrix3x3)(vw::cartography::GeoReference::PixelInterpretation))
  MVP_WRAP(lonlat_to_pixel, (vw::Vector2)(vw::Vector2))
  MVP_WRAP(pixel_to_lonlat, (vw::Vector2)(vw::Vector2))
  MVP_WRAP(datum, (vw::cartography::Datum const&))
END_MVP_WRAPPER()
