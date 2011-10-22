/// \file OrbitalImageFootprint.h
///
/// Orbital Image Footprint Class class
///
/// TODO: Write me!
/// The OrbitalImageFile class represents a single orbital image and its camera
/// model saved on disk. Also, it can determine if the footprint of the orbital 
/// image intersects a given lonlat BBox using the intersects() function.

#ifndef __MVP_ORBITALIMAGEFOOTPRINT_H__
#define __MVP_ORBITALIMAGEFOOTPRINT_H__

#include <mvp/OrbitalImageFileDescriptor.pb.h>

#include <vw/Cartography/Datum.h>
#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/FileIO/DiskImageResource.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
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

class OrbitalImageFootprint
{
  OrbitalImageFileDescriptor m_image_file;
  vw::Vector2i m_image_size;
  std::vector<vw::Vector2> m_footprint;

  public:

    OrbitalImageFootprint(std::string const& image_path, std::string const& camera_path, vw::cartography::Datum const& datum, vw::Vector2 const& post_height_limits)
    {
      m_image_file.set_camera_path(camera_path);
      m_image_file.set_image_path(image_path);

      boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(m_image_file.image_path()));
      m_image_size.x() = rsrc->cols();
      m_image_size.y() = rsrc->rows();

      // TODO: Rewrite for ellipsoid
      VW_ASSERT(datum.semi_major_axis() == datum.semi_minor_axis(),
        vw::ArgumentErr() << "Datum must be spheroid");

      vw::Vector2 radius_range = vw::Vector2(datum.radius(0, 0), datum.radius(0, 0)) + post_height_limits;
      m_footprint = construct_footprint(m_image_file.camera_path(), m_image_size, radius_range);
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

    OrbitalImageFileDescriptor orbital_image_file() const {
      return m_image_file;
    }

    /// Return a lonlat bounding box for the footprint of the orbital image
    vw::BBox2 lonlat_bbox() const {
      vw::BBox2 bbox;

      BOOST_FOREACH(vw::Vector2 v, m_footprint) {
        bbox.grow(v);
      }

      return bbox;
    }

    /// Return the level for which the resolution of one tile at that level
    /// is approximately equal to the resolution of the orbital image.
    int equal_resolution_level() const {
      vw::BBox2 fp_bbox(lonlat_bbox());

      double x_res_lvl = log(360.0 / fp_bbox.width()) / log(2);
      double y_res_lvl = log(360.0 / fp_bbox.height()) / log(2);

      return round(std::max(x_res_lvl, y_res_lvl));
    }

    /// Return the level for which the pixel density (measured as pixels per 
    /// degree) at that level is greater or equal to the pixel density of
    /// the orbital image.
    int equal_density_level(int tile_size) const {
      vw::BBox2 fp_bbox(lonlat_bbox());

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

class OrbitalImageFootprintCollection : public std::vector<OrbitalImageFootprint> {
  vw::cartography::Datum m_datum;
  vw::Vector2 m_post_height_limits;

  public:
    OrbitalImageFootprintCollection(vw::cartography::Datum const& datum, vw::Vector2 const& post_height_limits) :
      m_datum(datum), m_post_height_limits(post_height_limits) {}

    OrbitalImageFootprintCollection(vw::cartography::Datum const& datum, double post_height_limit_min, double post_height_limit_max) :
      m_datum(datum), m_post_height_limits(vw::Vector2(post_height_limit_min, post_height_limit_max)) {}

    void add_image(std::string const& image_path, std::string const& camera_path) {
      this->push_back(OrbitalImageFootprint(image_path, camera_path, m_datum, m_post_height_limits));
    }

    void add_image_pattern(std::string const& image_pattern, std::string const& camera_pattern, int start, int end) {
      namespace fs = boost::filesystem;

      for (int i = start; i <= end; i++) {
        std::string camera_file = (boost::format(camera_pattern) % i).str();
        std::string image_file = (boost::format(image_pattern) % i).str();
        if (fs::exists(camera_file) && fs::exists(image_file)) {
          add_image(image_file, camera_file);
        } else {
          vw::vw_out(vw::DebugMessage, "mvp") << "Couldn't find " << camera_file << " or " << image_file;
        }
      }
    }

    int equal_resolution_level() const {
      int result = std::numeric_limits<int>::max();

      BOOST_FOREACH(OrbitalImageFootprint const& fp, *this) {
        result = std::min(result, fp.equal_resolution_level());
      }

      return result;
    }

    int equal_density_level(int tile_size) const {
      int result = 0;

      BOOST_FOREACH(OrbitalImageFootprint const& fp, *this) {
        result = std::max(result, fp.equal_density_level(tile_size));
      }

      return result;
    }

    vw::BBox2 lonlat_bbox() const {
      vw::BBox2 result;

      BOOST_FOREACH(OrbitalImageFootprint const& fp, *this) {
        result.grow(fp.lonlat_bbox());
      }

      return result;
    }

    std::vector<OrbitalImageFileDescriptor> images_in_region(vw::BBox2 const& lonlat_bbox) const {
      std::vector<OrbitalImageFileDescriptor> result;

      BOOST_FOREACH(OrbitalImageFootprint const& fp, *this) {
        if (fp.intersects(lonlat_bbox)) {
          result.push_back(fp.orbital_image_file());
        } 
      }

      return result;
    }
};

} // namespace mvp

#endif
