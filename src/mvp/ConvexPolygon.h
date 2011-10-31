/// \file ConvexPolygon.h
///
/// Convex Polygon Class
///
/// TODO: Write me!

#ifndef __MVP_CONVEXPOLYGON_H__
#define __MVP_CONVEXPOLYGON_H__

#include <vw/Math/Vector.h>

#include <boost/foreach.hpp>

namespace mvp {


typedef std::vector<vw::Vector2> ConvexPolygon;

/// Return the circulation direction of three points. Negative if clockwise,
/// positive if anticlockwise, zero if colinear.
int circulation_direction(vw::Vector2 const& v0, vw::Vector2 const& v1, vw::Vector2 const& v) {
  return (v.y() - v0.y()) * (v1.x() - v0.x()) - (v.x() - v0.x()) * (v1.y() - v0.y());
}

bool isect_poly(ConvexPolygon poly1, ConvexPolygon poly2) {
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
