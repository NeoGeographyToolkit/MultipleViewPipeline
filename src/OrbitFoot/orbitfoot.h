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

class OrbitFootprint
{
  std::string m_camfile;
  vw::Vector2i m_img_size;
  std::vector<vw::Vector2> m_vertices;

  public:
    OrbitFootprint(std::string const& camfile, 
                   vw::Vector2i const& img_size, 
                   double sphere_rad) :
      m_camfile(camfile), m_img_size(img_size), m_vertices(4)
    {
      vw::camera::PinholeModel cam(camfile);

      // Vertices are given clockwise
      m_vertices[0] = backproj_px(cam, vw::Vector2i(0, 0), sphere_rad);
      m_vertices[1] = backproj_px(cam, vw::Vector2i(img_size.x(), 0), sphere_rad);
      m_vertices[2] = backproj_px(cam, vw::Vector2i(img_size.x(), img_size.y()), sphere_rad);
      m_vertices[3] = backproj_px(cam, vw::Vector2i(0, img_size.y()), sphere_rad);
    }

    bool intersects(vw::BBox2 tile) {
      return true;
    }
};

#endif
