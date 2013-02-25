#include <mvp/Octave/MvpWrapper.h>
#include <vw/Cartography/GeoReference.h>

BEGIN_MVP_WRAPPER(GeoReference, vw::cartography::GeoReference)
  MVP_WRAP_CONSTRUCTOR((vw::cartography::Datum))
  MVP_WRAP(lonlat_to_pixel)
  MVP_WRAP(pixel_to_lonlat)
END_MVP_WRAPPER()
