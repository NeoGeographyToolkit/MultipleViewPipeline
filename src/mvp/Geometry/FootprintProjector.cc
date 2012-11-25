#include <mvp/Geometry/FootprintProjector.h>

#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

#include <boost/foreach.hpp>

namespace mvp {
namespace geometry {

vw::Vector2 FootprintProjector::forward(vw::Vector2 const& lonlat, double alt) {
  return m_camera->point_to_pixel(m_datum.geodetic_to_cartesian(vw::Vector3(lonlat.x(), lonlat.y(), alt)));
}

vw::Vector2 FootprintProjector::backward(vw::Vector2 const& px, double alt) {
  VW_ASSERT(m_datum.semi_major_axis() == m_datum.semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 

  boost::shared_ptr<vw::camera::PinholeModel> pin_camera = boost::dynamic_pointer_cast<vw::camera::PinholeModel>(m_camera);
  VW_ASSERT(pin_camera, vw::NoImplErr() << "Only pinhole cameras supported");

  double sphere_rad = m_datum.semi_major_axis() + alt;

  vw::Vector3 dir = pin_camera->pixel_to_vector(px);
  vw::Vector3 cntr = pin_camera->camera_center();

  double a = dot_prod(dir, dir);
  double b = 2 * dot_prod(cntr, dir);
  double c = dot_prod(cntr, cntr) - sphere_rad * sphere_rad;

  double t = (-b - sqrt(b*b-4*a*c))/2/a;

  vw::Vector3 isect = cntr + t * dir;

  vw::Vector3 llr = vw::cartography::xyz_to_lon_lat_radius(isect);

  return vw::math::subvector(llr, 0, 2);
}

ConvexPolygon FootprintProjector::forward(ConvexPolygon const& poly, vw::Vector2 const& alt_limits) {
  std::vector<vw::Vector2> result;
  BOOST_FOREACH(vw::Vector2 const& v, poly.vertices()) {
      result.push_back(forward(v, alt_limits[0]));
      result.push_back(forward(v, alt_limits[1]));
  }
  return ConvexPolygon(result);
}

ConvexPolygon FootprintProjector::backward(ConvexPolygon const& poly, vw::Vector2 const& alt_limits) {
  std::vector<vw::Vector2> result;
  BOOST_FOREACH(vw::Vector2 const& v, poly.vertices()) {
      result.push_back(backward(v, alt_limits[0]));
      result.push_back(backward(v, alt_limits[1]));
  }
  return ConvexPolygon(result);
}

}} // geometry, mvp
