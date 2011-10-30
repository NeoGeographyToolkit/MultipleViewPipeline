/// \file PolygonMath.h
///
/// Orbital Image Footprint Class class
///
/// TODO: Write me!

#ifndef __MVP_POLYGONMATH_H__
#define __MVP_POLYGONMATH_H__

#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

#include <boost/foreach.hpp>

namespace mvp {

vw::Vector2 backproj_px(vw::camera::PinholeModel const& cam, vw::Vector2 const& px, double sphere_rad)
{
  vw::Vector3 dir = cam.pixel_to_vector(px);
  vw::Vector3 cntr = cam.camera_center();

  double a = dot_prod(dir, dir);
  double b = 2 * dot_prod(cntr, dir);
  double c = dot_prod(cntr, cntr) - sphere_rad * sphere_rad;

  double t = (-b - sqrt(b*b-4*a*c))/2/a;

  vw::Vector3 isect = cntr + t * dir;

  vw::Vector3 llr = vw::cartography::xyz_to_lon_lat_radius(isect);

  return vw::math::subvector(llr, 0, 2);
}

bool isect_poly(std::vector<vw::Vector2> poly1, std::vector<vw::Vector2> poly2)
{
  // Algorithm from http://www.gpwiki.org/index.php/Polygon_Collision
  // Straight up dumb check, no optimizations attempted...
  VW_ASSERT(poly1.size() >= 3 && poly2.size() >= 3, vw::LogicErr() << "Invalid polygons");


  for (int currVert1 = 0; currVert1 < poly1.size(); currVert1++) {
    int nextVert1 = currVert1 + 1;
    nextVert1 = nextVert1 < poly1.size() ? nextVert1: 0;

    vw::Vector2 dir = poly1[nextVert1] - poly1[currVert1];
    vw::Vector2 perp_dir(-dir[1], dir[0]);

    double poly1_min = std::numeric_limits<double>::max();
    double poly1_max = std::numeric_limits<double>::min();
    BOOST_FOREACH(vw::Vector2 v, poly1) {
      double res = dot_prod(perp_dir, v);
      poly1_min = std::min(res, poly1_min);
      poly1_max = std::max(res, poly1_max);
    }

    double poly2_min = std::numeric_limits<double>::max();
    double poly2_max = std::numeric_limits<double>::min();
    BOOST_FOREACH(vw::Vector2 v, poly2) {
      double res = dot_prod(perp_dir, v);
      poly2_min = std::min(res, poly2_min);
      poly2_max = std::max(res, poly2_max);
    }

    if (poly1_min > poly2_max || poly1_max < poly2_min) {
      return false;
    }
  }

  return true;  
}

} // namespace mvp

#endif
