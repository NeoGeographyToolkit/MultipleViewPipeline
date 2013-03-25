#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Image/OrbitalImageCollection.h>

BEGIN_MVP_WRAPPER(OrbitalImageCollection, mvp::image::OrbitalImageCollection, MVP_WRAPPER_NO_ARGS)
  MVP_WRAPPER(back_project, (std::vector<vw::ImageView<mvp::image::OrbitalImagePixel> >)
                            (vw::Vector3 const&)(vw::Quat const&)(vw::Vector2 const&)(vw::Vector2i const&))
END_MVP_WRAPPER()
