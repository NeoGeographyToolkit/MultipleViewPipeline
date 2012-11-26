#include <mvp/Image/OrbitalImageCollection.h>

#include <boost/foreach.hpp>

namespace mvp {
namespace image {

std::vector<vw::ImageView<OrbitalData> > OrbitalImageCollection::back_project(vw::Vector3 const& xyz, 
                                                                              vw::Quat const& orientation, 
                                                                              vw::Vector2 const& scale,
                                                                              vw::Vector2i const& size)
{
  std::vector<vw::ImageView<OrbitalData> > result;

  BOOST_FOREACH(OrbitalImage const& o, *this) {
    vw::ImageView<OrbitalData> patch = o.back_project(xyz, orientation, scale, size);
    if (!vw::is_transparent(patch)) {
      result.push_back(patch);
    }
  }  

  return result;
}

}} // namespace image,mvp
