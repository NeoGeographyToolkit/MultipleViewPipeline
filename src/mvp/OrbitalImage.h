/// \file OrbitalImage.h
///
/// Orbital Image class
///
/// The OrbitalImage class represents a single orbital image and its camera
/// model. Also, it can determine if the footprint of the orbital image
/// intersects a given lonlat BBox using the intersects() function.

#ifndef __ORBITALIMAGE_H__
#define __ORBITALIMAGE_H__

namespace mvp {

class OrbitalImage
{
  std::string m_camera_path, m_image_path;
  vw::Vector2i m_image_size; 

  public:

    OrbitalImage(std::string const& camera_path, std::string const& image_path) :
      m_camera_path(camera_path), m_image_path(image_path) 
    {
      boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
      m_image_size.x() = rsrc.cols();
      m_image_size.y() = rsrc.rows();

      // TODO: Construct footprint
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
