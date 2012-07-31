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
#include <vw/Camera/PinholeModel.h>

#include <boost/foreach.hpp>

namespace mvp {

class OrbitalImageFootprint : public ConvexPolygon
{
  OrbitalImageFileDescriptor m_image_file;
  vw::Vector2i m_image_size;

  public:
    static vw::Vector2 backproj_px(vw::camera::PinholeModel const& cam, 
                                   vw::Vector2 const& px, 
                                   vw::cartography::Datum const& datum, 
                                   double alt);

    static OrbitalImageFootprint construct_from_paths(std::string const& image_path, 
                                                      std::string const& camera_path, 
                                                      vw::cartography::Datum const& datum, 
                                                      vw::Vector2 const& alt_limits);


    OrbitalImageFileDescriptor orbital_image_file() const {return m_image_file;}

    /// Return the level for which the resolution of one tile at that level
    /// is approximately equal to the resolution of the orbital image.
    int equal_resolution_level() const; 

    /// Return the level for which the pixel density (measured as pixels per 
    /// degree) at that level is greater or equal to the pixel density of
    /// the orbital image.
    int equal_density_level(int tile_size) const;

  protected:
    // Make sure the user doesn't construct one
    OrbitalImageFootprint(OrbitalImageFileDescriptor const& image_file, vw::Vector2i const& image_size, VertexList point_list) :
      ConvexPolygon(point_list), m_image_file(image_file), m_image_size(image_size) {}
};

class OrbitalImageFootprintCollection : public std::vector<OrbitalImageFootprint> {
  vw::cartography::Datum m_datum;
  vw::Vector2 m_alt_limits;

  public:
    OrbitalImageFootprintCollection(vw::cartography::Datum const& datum, vw::Vector2 const& alt_limits) :
      m_datum(datum), m_alt_limits(alt_limits) {}

    OrbitalImageFootprintCollection(vw::cartography::Datum const& datum, double alt_limit_min, double alt_limit_max) :
      m_datum(datum), m_alt_limits(alt_limit_min, alt_limit_max) {}

    void add_image(OrbitalImageFileDescriptor const& image_file) {
      add_image(image_file.image_path(), image_file.camera_path());
    }

    void add_image(std::string const& image_path, std::string const& camera_path) {
      push_back(OrbitalImageFootprint::construct_from_paths(image_path, camera_path, m_datum, m_alt_limits));
    }

    template <class CollectionT>
    void add_image_collection(CollectionT const& orbital_images) {
      BOOST_FOREACH(OrbitalImageFileDescriptor const& o, orbital_images) {
        add_image(o);
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
