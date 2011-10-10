#ifndef __ORBIT_FOOT_H__
#define __ORBIT_FOOT_H__

#include <boost/foreach.hpp>

#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

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

class OrbitFootprint
{
  const static int m_tile_size = 256;
  std::string m_camfile;
  vw::Vector2i m_img_size;
  std::vector<vw::Vector2> m_vertices;
  vw::BBox2 m_bbox;

  public:
    OrbitFootprint(std::string const& camfile, 
                   vw::Vector2i const& img_size, 
                   double sphere_rad) :
      m_camfile(camfile), m_img_size(img_size), m_vertices(4), m_bbox()
    {
      vw::camera::PinholeModel cam(camfile);

      // Vertices are given clockwise
      m_vertices[0] = backproj_px(cam, vw::Vector2i(0, 0), sphere_rad);
      m_vertices[1] = backproj_px(cam, vw::Vector2i(img_size.x(), 0), sphere_rad);
      m_vertices[2] = backproj_px(cam, vw::Vector2i(img_size.x(), img_size.y()), sphere_rad);
      m_vertices[3] = backproj_px(cam, vw::Vector2i(0, img_size.y()), sphere_rad);

      // Create a BBox for the footprint
      BOOST_FOREACH(vw::Vector2 v, m_vertices) {
        m_bbox.grow(v);
      }

      // TODO: Take care of wrapping around the poles!
      // For now just warn
      //
      // also, right now the footprints are being stored upside-down...
      vw::BBox2 bounds(-180,-180,360,360);
      BOOST_FOREACH(vw::Vector2 v, m_vertices) {
        if (!bounds.contains(v)) {
          std::cout << v << " Ooops this vertex will wrap! fixme!" << std::endl;
        }
      }
    }

    std::string camfile() const {
      return m_camfile;
    }

    vw::BBox2 bounding_box() const {
      return m_bbox;
    }

    bool intersects(vw::BBox2 tile) const {
      std::vector<vw::Vector2> tile_poly(4);
      tile_poly[0] = tile.min();
      tile_poly[1] = vw::Vector2(tile.min().x(), tile.max().y());
      tile_poly[2] = tile.max();
      tile_poly[3] = vw::Vector2(tile.max().x(), tile.min().y());

      return isect_poly(m_vertices, tile_poly);
    }

    bool intersects(int col, int row, int level) const {
      vw::BBox2 tile_box(col * m_tile_size, row * m_tile_size, m_tile_size, m_tile_size);

      // Resolution: size of workspace, in pixels
      double resolution = m_tile_size << level;

      // Create a transform
      vw::Matrix3x3 transform;
      transform(0,0) = 360.0 / resolution;
      transform(0,2) = -180.0;
      transform(1,1) = -360.0 / resolution;
      transform(1,2) = 180.0;
      transform(2,2) = 1;

      vw::BBox2 newbox;
      newbox.grow(dehom(transform * hom(tile_box.min())));
      newbox.grow(dehom(transform * hom(tile_box.max())));

      return this->intersects(newbox);
    }
};

#endif
