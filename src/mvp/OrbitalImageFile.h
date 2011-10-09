/// \file OrbitalImage.h
///
/// Orbital Image Class class
///
/// The OrbitalImageFile class represents a single orbital image and its camera
/// model saved on disk. Also, it can determine if the footprint of the orbital 
/// image intersects a given lonlat BBox using the intersects() function.

#ifndef __ORBITALIMAGEFILE_H__
#define __ORBITALIMAGEFILE_H__

#include <mvp/OrbitalImageFileDesc.pb.h>

#include <vw/Cartography/Datum.h>
#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/FileIO/DiskImageResource.h>

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

class OrbitalImageFile
{
  std::string m_camera_path, m_image_path;
  vw::Vector2i m_image_size;
  vw::Vector2 m_radius_range;
  std::vector<vw::Vector2> m_footprint;

  public:

    OrbitalImageFile(std::string const& camera_path, std::string const& image_path, vw::Vector2 const& radius_range) :
      m_camera_path(camera_path), m_image_path(image_path), m_radius_range(radius_range)
    {
      boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
      m_image_size.x() = rsrc->cols();
      m_image_size.y() = rsrc->rows();

      m_footprint = construct_footprint(m_camera_path, m_image_size, radius_range);
    }

    OrbitalImageFile(std::string const& camera_path, vw::Vector2i const& image_size, vw::Vector2 const& radius_range) :
      m_camera_path(camera_path), m_image_path("None"), m_image_size(image_size), m_radius_range(radius_range)

    {
      m_footprint = construct_footprint(m_camera_path, m_image_size, radius_range);
    }

    /// Construct an OrbitalImageFile from a OrbitalImageFileDesc
    OrbitalImageFile(OrbitalImageFileDesc const& desc) :
      m_camera_path(desc.camera_path()), m_image_path(desc.image_path()), 
      m_image_size(vw::Vector2i(desc.image_cols(), desc.image_rows())),
      m_radius_range(vw::Vector2(desc.radius_min(), desc.radius_max()))
    {
      VW_ASSERT(desc.footprint_lons_size() == desc.footprint_lats_size(), 
                vw::InputErr() << "Malformed footprint record");

      // TODO: Don't store footprint in protobuf?
      for (int i = 0; i < desc.footprint_lons_size(); i++) {
        m_footprint.push_back(vw::Vector2(desc.footprint_lons(i), desc.footprint_lats(i)));
      }
    }

    /// Create a OrbitalImageFileDesc that represents this OrbitalImageFile
    OrbitalImageFileDesc build_desc() const {
      OrbitalImageFileDesc desc;
      desc.set_camera_path(m_camera_path);
      desc.set_image_path(m_image_path);
      desc.set_image_cols(m_image_size.x());
      desc.set_image_rows(m_image_size.y());
      desc.set_radius_min(m_radius_range[0]);
      desc.set_radius_max(m_radius_range[1]);
      
      BOOST_FOREACH(vw::Vector2 vertex, m_footprint) {
        desc.add_footprint_lons(vertex.x());
        desc.add_footprint_lats(vertex.y());
      }

      return desc;
    }

    /// A static method that constructs a polygon that represents an orbital
    /// image's footprint
    static std::vector<vw::Vector2> construct_footprint(vw::camera::PinholeModel const& camera, vw::Vector2i const& image_size,
                                                        vw::Vector2 const& radius_range)
    {
      std::vector<vw::Vector2> fp;

      // TODO: Take into account the radius range!
      fp.push_back(backproj_px(camera, vw::Vector2i(0, 0), radius_range[0]));
      fp.push_back(backproj_px(camera, vw::Vector2i(image_size.x(), 0), radius_range[0]));
      fp.push_back(backproj_px(camera, vw::Vector2i(image_size.x(), image_size.y()), radius_range[0]));
      fp.push_back(backproj_px(camera, vw::Vector2i(0, image_size.y()), radius_range[0]));

      // TODO: Need to take care of wrapping around poles!
      vw::BBox2 maxbounds(-180,-180,360,360);
      BOOST_FOREACH(vw::Vector2 v, fp) {
        if (!maxbounds.contains(v)) {
          VW_ASSERT(maxbounds.contains(v), vw::NoImplErr() << "Lonlat wrapping in footprints needs to be implemented");
        }
      }

      return fp;
    }

    /// Return a lonlat bounding box for the footprint of the orbital image
    vw::BBox2 footprint_bbox() const {
      vw::BBox2 bbox;

      BOOST_FOREACH(vw::Vector2 v, m_footprint) {
        bbox.grow(v);
      }

      return bbox;
    }

    /// Reset the radius range for the footprint
    void set_radius_range(vw::Vector2 const& radius_range) {
      m_radius_range = radius_range;
      m_footprint = construct_footprint(m_camera_path, m_image_size, radius_range);
    }

    /// Return the level for which the resolution of one tile at that level
    /// is approximately equal to the resolution of the orbital image.
    int equal_resolution_level() const {
      vw::BBox2 fp_bbox(footprint_bbox());

      double x_res_lvl = log(360.0 / fp_bbox.width()) / log(2);
      double y_res_lvl = log(360.0 / fp_bbox.height()) / log(2);

      return round(std::max(x_res_lvl, y_res_lvl));
    }

    /// Return the level for which the pixel density (measured as pixels per 
    /// degree) at that level is greater or equal to the pixel density of
    /// the orbital image.
    int equal_density_level(int tile_size = 256) const {
      vw::BBox2 fp_bbox(footprint_bbox());

      double x_dens_lvl = log(360.0 * m_image_size.x() / fp_bbox.width() / tile_size) / log(2);
      double y_dens_lvl = log(360.0 * m_image_size.y() / fp_bbox.height() / tile_size) / log(2);

      return ceil(std::max(x_dens_lvl, y_dens_lvl));
    }

    /// Return true if the footprint of the orbital image intersects the given
    /// lonlat BBox
    bool intersects(vw::BBox2 lonlat_bbox) const {
      std::vector<vw::Vector2> bbox_poly(4);
      bbox_poly[0] = lonlat_bbox.min();
      bbox_poly[1] = vw::Vector2(lonlat_bbox.min().x(), lonlat_bbox.max().y());
      bbox_poly[2] = lonlat_bbox.max();
      bbox_poly[3] = vw::Vector2(lonlat_bbox.max().x(), lonlat_bbox.min().y());

      return isect_poly(m_footprint, bbox_poly);
    }

};

typedef std::vector<OrbitalImageFile> OrbitalImageFileCollection;

} // namespace mvp

#endif
