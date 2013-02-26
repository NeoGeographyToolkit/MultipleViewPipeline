#include <mvp/Octave/MvpWrapper.h>
#include <vw/Cartography/GeoReference.h>

BEGIN_MVP_WRAPPER(GeoReference, vw::cartography::GeoReference)
  MVP_WRAP_CONSTRUCTOR((vw::cartography::Datum))
  MVP_WRAP_CONSTRUCTOR((vw::cartography::Datum)(vw::Matrix3x3))
  MVP_WRAP_CONSTRUCTOR((vw::cartography::Datum)(vw::Matrix3x3)(vw::cartography::GeoReference::PixelInterpretation))
  MVP_WRAP_FUNCTION((lonlat_to_pixel)(vw::Vector2))
  MVP_WRAP_FUNCTION((pixel_to_lonlat)(vw::Vector2))
  MVP_WRAP_FUNCTION((datum))
END_MVP_WRAPPER()
