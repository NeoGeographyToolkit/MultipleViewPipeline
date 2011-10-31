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
#include <mvp/ConvexPolygon.h>

#include <vw/Cartography/Datum.h>
#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/FileIO/DiskImageResource.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace mvp {

vw::Vector2 backproj_px(vw::camera::PinholeModel const& cam, vw::Vector2 const& px, vw::cartography::Datum const& datum, double alt) {
  VW_ASSERT(datum.semi_major_axis() == datum.semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 

  double sphere_rad = datum.semi_major_axis() + alt;

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

class OrbitalImageFootprint : public ConvexPolygon
{
  OrbitalImageFileDescriptor m_image_file;
  vw::Vector2i m_image_size;

  public:
    static OrbitalImageFootprint construct_from_paths(std::string const& image_path, 
                                                      std::string const& camera_path, 
                                                      vw::cartography::Datum const& datum, 
                                                      vw::Vector2 const& post_height_limits)
    {
      OrbitalImageFileDescriptor image_file;
      image_file.set_camera_path(camera_path);
      image_file.set_image_path(image_path);

      boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_file.image_path()));
      vw::Vector2i image_size(rsrc->cols(), rsrc->rows());

      std::vector<vw::Vector2> fp_points(4);

      vw::camera::PinholeModel camera(image_file.camera_path());
      fp_points[0] = backproj_px(camera, vw::Vector2i(0, 0), datum, post_height_limits[0]);
      fp_points[1] = backproj_px(camera, vw::Vector2i(image_size.x(), 0), datum, post_height_limits[0]);
      fp_points[2] = backproj_px(camera, vw::Vector2i(image_size.x(), image_size.y()), datum, post_height_limits[0]);
      fp_points[3] = backproj_px(camera, vw::Vector2i(0, image_size.y()), datum, post_height_limits[0]);

      // TODO: Need to take care of wrapping around poles!
      vw::BBox2 maxbounds(-180,-180,360,360);
      BOOST_FOREACH(vw::Vector2 v, fp_points) {
        if (!maxbounds.contains(v)) {
          VW_ASSERT(maxbounds.contains(v), vw::NoImplErr() << "Lonlat wrapping in footprints needs to be implemented");
        }
      }

      return OrbitalImageFootprint(image_file, image_size, fp_points);
    }

    OrbitalImageFileDescriptor orbital_image_file() const {
      return m_image_file;
    }

    /// Return the level for which the resolution of one tile at that level
    /// is approximately equal to the resolution of the orbital image.
    int equal_resolution_level() const {
      vw::BBox2 fp_bbox(bounding_box());

      double x_res_lvl = log(360.0 / fp_bbox.width()) / log(2);
      double y_res_lvl = log(360.0 / fp_bbox.height()) / log(2);

      return round(std::max(x_res_lvl, y_res_lvl));
    }

    /// Return the level for which the pixel density (measured as pixels per 
    /// degree) at that level is greater or equal to the pixel density of
    /// the orbital image.
    int equal_density_level(int tile_size) const {
      vw::BBox2 fp_bbox(bounding_box());

      double x_dens_lvl = log(360.0 * m_image_size.x() / fp_bbox.width() / tile_size) / log(2);
      double y_dens_lvl = log(360.0 * m_image_size.y() / fp_bbox.height() / tile_size) / log(2);

      return ceil(std::max(x_dens_lvl, y_dens_lvl));
    }

  protected:
    // Make sure the user doesn't construct one
    template <class ContainerT>
    OrbitalImageFootprint(OrbitalImageFileDescriptor const& image_file, vw::Vector2i const& image_size, ContainerT point_list) :
      ConvexPolygon(point_list), m_image_file(image_file), m_image_size(image_size) {}
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
      this->push_back(OrbitalImageFootprint::construct_from_paths(image_path, camera_path, m_datum, m_post_height_limits));
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
        result.grow(fp.bounding_box());
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
