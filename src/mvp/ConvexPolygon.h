/// \file ConvexPolygon.h
///
/// Convex Polygon Class
///
/// TODO: Write me!

#ifndef __MVP_CONVEXPOLYGON_H__
#define __MVP_CONVEXPOLYGON_H__

#include <vw/Math/Vector.h>
#include <vw/Math/BBox.h>

#include <boost/foreach.hpp>

namespace mvp {

/// Return the circulation direction of three points. Negative if clockwise,
/// positive if anticlockwise, zero if colinear.
int circulation_direction(vw::Vector2 const& v0, vw::Vector2 const& v1, vw::Vector2 const& v) {
  return (v.y() - v0.y()) * (v1.x() - v0.x()) - (v.x() - v0.x()) * (v1.y() - v0.y());
}

struct ConvexPolygon {
  typedef std::vector<vw::Vector2> VertexList;

  private:

    VertexList m_vertices;

  public:

    ConvexPolygon() : m_vertices() {}

    // TODO: Always find convex hull of input
    template <class ContainerT>
    ConvexPolygon(ContainerT vertices) : m_vertices(vertices) {}

    vw::BBox2 bounding_box() const {
      vw::BBox2 bbox;

      BOOST_FOREACH(vw::Vector2 const& v, m_vertices) {
        bbox.grow(v);
      }

      return bbox;
    }

    bool intersects(ConvexPolygon const& other) const {
      // Algorithm from http://www.gpwiki.org/index.php/Polygon_Collision
      // Straight up dumb check, no optimizations attempted...

      for (VertexList::const_iterator cursor = m_vertices.begin(); cursor != m_vertices.end(); cursor++) {
        VertexList::const_iterator next = cursor;
        if (++next == m_vertices.end()) {
          next = m_vertices.begin();
        }

        vw::Vector2 dir = *next - *cursor;
        vw::Vector2 perp_dir(-dir[1], dir[0]);

        double poly1_min = std::numeric_limits<double>::max();
        double poly1_max = std::numeric_limits<double>::min();
        BOOST_FOREACH(vw::Vector2 v, m_vertices) {
          double res = dot_prod(perp_dir, v);
          poly1_min = std::min(res, poly1_min);
          poly1_max = std::max(res, poly1_max);
        }

        double poly2_min = std::numeric_limits<double>::max();
        double poly2_max = std::numeric_limits<double>::min();
        BOOST_FOREACH(vw::Vector2 v, other.m_vertices) {
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

    bool intersects(vw::BBox2 bbox) const {
      ConvexPolygon bbox_poly;
      
      std::vector<vw::Vector2> bbox_verts(4);
      bbox_verts[0] = bbox.min();
      bbox_verts[1] = vw::Vector2(bbox.min().x(), bbox.max().y());
      bbox_verts[2] = bbox.max();
      bbox_verts[3] = vw::Vector2(bbox.max().x(), bbox.min().y());

      bbox_poly.m_vertices = bbox_verts;

      return intersects(bbox_poly);
    }
};

} // namespace mvp

#endif
