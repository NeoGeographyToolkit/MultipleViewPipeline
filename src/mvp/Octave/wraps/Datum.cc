#include <mvp/Octave/MvpWrapper.h>
#include <vw/Cartography/Datum.h>

BEGIN_MVP_WRAPPER(Datum, vw::cartography::Datum, MVP_WRAPPER_NO_ARGS)
  MVP_WRAPPER_CONSTRUCTOR((std::string))
  MVP_WRAPPER(semi_major_axis, (double))
  MVP_WRAPPER(semi_minor_axis, (double))
END_MVP_WRAPPER()
