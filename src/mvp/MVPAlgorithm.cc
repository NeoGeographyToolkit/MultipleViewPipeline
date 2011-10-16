#include <mvp/MVPAlgorithm.h>

#include <boost/foreach.hpp>

#include <vw/Cartography/SimplePointImageManipulation.h>

using namespace vw;

namespace mvp {

const MVPAlgorithmResult MVPAlgorithmTestImpl::do_algorithm(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef) const {
  Vector2 ll = georef.pixel_to_lonlat(Vector2(0, 0));
  Vector3 llr(ll[0], ll[1], georef.datum().radius(ll[0], ll[1]));
  Vector3 xyz = cartography::lon_lat_radius_to_xyz(llr);

  int overlaps = 0;
  BOOST_FOREACH(OrbitalImageCrop const& o, m_images) {
    Vector2 px = o.camera().point_to_pixel(xyz);
    if (bounding_box(o.image()).contains(px)) {
      overlaps++;
    }
  }

  MVPAlgorithmVar result(overlaps, Vector3f(overlaps, overlaps, overlaps), Vector3f(overlaps, overlaps, overlaps));

  return MVPAlgorithmResult(result, overlaps, true, overlaps);
}

} // namespace mvp
