#include <mvp/Octave/MvpWrapper.h>
#include <vw/Cartography/Datum.h>

BEGIN_MVP_WRAPPER(Datum, vw::cartography::Datum, MVP_WRAPPER_NO_ARGS)
  MVP_WRAPPER_CONSTRUCTOR((std::string))
  MVP_WRAPPER_C(semi_major_axis, (double))
  MVP_WRAPPER_C(semi_minor_axis, (double))
  MVP_WRAPPER_C(geodetic_to_cartesian, (vw::Vector3)(vw::Vector3 const&))
END_MVP_WRAPPER()
