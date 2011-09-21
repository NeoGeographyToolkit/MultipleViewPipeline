/// \file OrbitalImage.h
///
/// Orbital Image class
///
/// The OrbitalImage class represents a single orbital image and its camera
/// model. Also, it can determine if the footprint of the orbital image
/// intersects a given lonlat BBox using the intersects() function.

#ifndef __ORBITALIMAGE_H__
#define __ORBITALIMAGE_H__

#include "OrbitalImageDesc.pb.h"

#include <vw/Cartography/Datum.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/FileIO/DiskImageResource.h>

namespace mvp {

class OrbitalImage
{
  std::string m_camera_path, m_image_path;
  vw::Vector2i m_image_size;
  vw::Vector2 m_radius_range;
  std::vector<vw::Vector2> m_footprint;

  public:

    OrbitalImage(std::string const& camera_path, std::string const& image_path, vw::Vector2 const& radius_range) :
      m_camera_path(camera_path), m_image_path(image_path), m_radius_range(radius_range)
    {
      boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
      m_image_size.x() = rsrc->cols();
      m_image_size.y() = rsrc->rows();

      m_footprint = construct_footprint(m_camera_path, m_image_size, radius_range);
    }

    OrbitalImage(std::string const& camera_path, vw::Vector2i const& image_size, vw::Vector2 const& radius_range) :
      m_camera_path(camera_path), m_image_path("None"), m_image_size(image_size), m_radius_range(radius_range)

    {
      m_footprint = construct_footprint(m_camera_path, m_image_size, radius_range);
    }

    OrbitalImage(OrbitalImageDesc const& desc) {

    }

    /// A static method that constructs a polygon that represents an orbital
    /// image's footprint
    static std::vector<vw::Vector2> construct_footprint(vw::camera::PinholeModel const& camera, vw::Vector2i const& image_size,
                                                        vw::Vector2 const& radius_range)
    {
      return std::vector<vw::Vector2>();
    }

    /// Reset the radius range for the footprint
    void set_radius_range(vw::Vector2 const& radius_range) {

    }

    /// Return the level for which the resolution of one tile at that level
    /// is approximately equal to the resolution of the orbital image.
    int equal_resolution_level() const {
      return 0;
    }

    /// Return the level for which the pixel density (measured as pixels per 
    /// degree) at that level is approximately equal to the pixel density of
    /// the orbital image.
    int equal_density_level() const {
      return 0;
    }

    /// Return true if the footprint of the orbital image intersects the given
    /// lonlat BBox
    bool intersects(vw::BBox2 lonlat_bbox) const {
      return false;
    }

};

typedef std::vector<OrbitalImage> OrbitalImageCollection;

} // namespace mvp

#endif
