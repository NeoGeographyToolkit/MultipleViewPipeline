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
/// positive if anticlockwise, zero if colinear. For Y pointing up, X pointing right
double circulation_direction(vw::Vector2 const& v0, vw::Vector2 const& v1, vw::Vector2 const& v) {
  return (v.y() - v0.y()) * (v1.x() - v0.x()) - (v.x() - v0.x()) * (v1.y() - v0.y());
}

struct ConvexPolygon {
  typedef std::vector<vw::Vector2> VertexList;

  private:

    VertexList m_vertices;

  public:

    ConvexPolygon() : m_vertices() {}

    template <class ContainerT>
    ConvexPolygon(ContainerT point_list) {
      // Algorithm is the 'giftwrap' algorithm http://www.cse.unsw.edu.au/~lambert/java/3d/giftwrap.html
      // Probably not the most efficient implementation...
      VW_ASSERT(point_list.size() >= 3, vw::ArgumentErr() << "Need at least 3 points to construct a polygon!");

      std::vector<vw::Vector2> pts(point_list);

      vw::Vector2 end = pts[0];
      BOOST_FOREACH(vw::Vector2 const& v, pts) {
        if (end.y() < v.y()) {
          end = v;
        }
      }

      bool done = false;
      vw::Vector2 curr = end;

      while (!done) {
        bool found_next = false;
        BOOST_FOREACH(vw::Vector2 const& next, pts) {
          if (curr != next) {
            found_next = true;
            BOOST_FOREACH(vw::Vector2 const& pt, pts) {
              if (circulation_direction(curr, next, pt) > 0 && pt != next) {
                found_next = false;
                break;
              }
            }
            if (found_next) {
              curr = next;
              break;
            }
          }
        }

        VW_ASSERT(found_next, vw::LogicErr() << "Unable to construct convex hull");

        m_vertices.push_back(curr);
        if (curr == end) {
          done = true;
        }
      }
    }

    VertexList vertices() const {
      return m_vertices;
    }

    vw::BBox2 bounding_box() const {
      vw::BBox2 bbox;

      BOOST_FOREACH(vw::Vector2 const& v, m_vertices) {
        bbox.grow(v);
      }

      return bbox;
    }

    bool contains(vw::Vector2 const& pt) const {
      // Use solution 3 from http://paulbourke.net/geometry/insidepoly/
      double prev_circulation = 0;
      for (VertexList::const_iterator curr = m_vertices.begin(); curr != m_vertices.end(); curr++) {
        VertexList::const_iterator next = curr;
        if (++next == m_vertices.end()) {
          next = m_vertices.begin();
        }

        double circulation = circulation_direction(*curr, *next, pt);

        if (prev_circulation * circulation < 0) {
          return false;
        }

        prev_circulation = circulation;
      }

      return true;
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
