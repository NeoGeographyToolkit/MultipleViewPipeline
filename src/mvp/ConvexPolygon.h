/// \file ConvexPolygon.h
///
/// Convex Polygon Class
///
/// TODO: Write me!

#ifndef __MVP_CONVEXPOLYGON_H__
#define __MVP_CONVEXPOLYGON_H__

#include <vw/Math/Vector.h>
#include <vw/Math/BBox.h>

namespace mvp {

struct ConvexPolygon {
  typedef std::vector<vw::Vector2> VertexList;

  private:
    VertexList m_vertices;

  public:
    ConvexPolygon() : m_vertices() {}

    ConvexPolygon(VertexList pts);

    VertexList vertices() const {return m_vertices;}

    vw::BBox2 bounding_box() const;

    /// Return the circulation direction of three points. Negative if clockwise,
    /// positive if anticlockwise, zero if colinear. For Y pointing up, X pointing right
    static double circulation_direction(vw::Vector2 const& v0, vw::Vector2 const& v1, vw::Vector2 const& v) {
      return (v.y() - v0.y()) * (v1.x() - v0.x()) - (v.x() - v0.x()) * (v1.y() - v0.y());
    }

    bool contains(vw::Vector2 const& pt) const;

    bool intersects(ConvexPolygon const& other) const;

    bool intersects(vw::BBox2 const& bbox) const;

};

} // namespace mvp

#endif
