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

#include <boost/foreach.hpp>

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

    /// Construct an OrbitalImage from a OrbitalImageDesc
    OrbitalImage(OrbitalImageDesc const& desc) :
      m_camera_path(desc.camera_path()), m_image_path(desc.image_path()), 
      m_image_size(vw::Vector2i(desc.image_cols(), desc.image_rows())),
      m_radius_range(vw::Vector2(desc.radius_min(), desc.radius_max()))
    {
      VW_ASSERT(desc.footprint_lons_size() == desc.footprint_lats_size(), 
                vw::InputErr() << "Malformed footprint record");
      for (int i = 0; i < desc.footprint_lons_size(); i++) {
        m_footprint.push_back(vw::Vector2(desc.footprint_lons(i), desc.footprint_lats(i)));
      }
    }

    /// Create a OrbitalImageDesc that represents this OrbitalImage
    OrbitalImageDesc build_desc() const {
      OrbitalImageDesc desc;
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

      return OrbitalImageDesc();
    }

    /// A static method that constructs a polygon that represents an orbital
    /// image's footprint
    static std::vector<vw::Vector2> construct_footprint(vw::camera::PinholeModel const& camera, vw::Vector2i const& image_size,
                                                        vw::Vector2 const& radius_range)
    {
      // TODO
      return std::vector<vw::Vector2>();
    }

    /// Reset the radius range for the footprint
    void set_radius_range(vw::Vector2 const& radius_range) {
      m_radius_range = radius_range;
      m_footprint = construct_footprint(m_camera_path, m_image_size, radius_range);
    }

    /// Return the level for which the resolution of one tile at that level
    /// is approximately equal to the resolution of the orbital image.
    int equal_resolution_level() const {
      // TODO
      return 0;
    }

    /// Return the level for which the pixel density (measured as pixels per 
    /// degree) at that level is approximately equal to the pixel density of
    /// the orbital image.
    int equal_density_level() const {
      // TODO
      return 0;
    }

    /// Return true if the footprint of the orbital image intersects the given
    /// lonlat BBox
    bool intersects(vw::BBox2 lonlat_bbox) const {
      // TODO
      return false;
    }

};

typedef std::vector<OrbitalImage> OrbitalImageCollection;

} // namespace mvp

#endif
