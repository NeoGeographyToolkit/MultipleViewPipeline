/// \file Footprint.h
///
/// Orbital Image Footprint Class
///
/// TODO: Write me!
///

#ifndef __MVP_ORBITALIMAGE_FOOTPRINT_H__
#define __MVP_ORBITALIMAGE_FOOTPRINT_H__

#include <mvp/OrbitalImage/ConvexPolygon.h>
#include <mvp/OrbitalImage/OrbitalImage.pb.h>

#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/Datum.h>

namespace mvp {
namespace orbitalimage {

class Footprint : public ConvexPolygon {
  OrbitalImage m_image;
  vw::Vector2i m_image_size;

  public:
    static vw::Vector2 backproj_px(vw::camera::PinholeModel const& cam, 
                                   vw::Vector2 const& px, 
                                   vw::cartography::Datum const& datum, 
                                   double alt);

    static Footprint construct_from_paths(std::string const& image_path, 
                                          std::string const& camera_path, 
                                          vw::cartography::Datum const& datum, 
                                          vw::Vector2 const& alt_limits);

    OrbitalImage orbital_image() const {return m_image;}

    /// Return the level for which the resolution of one tile at that level
    /// is approximately equal to the resolution of the orbital image.
    int equal_resolution_level() const; 

    /// Return the level for which the pixel density (measured as pixels per 
    /// degree) at that level is greater or equal to the pixel density of
    /// the orbital image.
    int equal_density_level(int tile_size) const;

  protected:
    // Make sure the user doesn't construct one
    Footprint(OrbitalImage const& image, vw::Vector2i const& image_size, VertexList point_list) :
      ConvexPolygon(point_list), m_image(image), m_image_size(image_size) {}

};

}} // namespace orbitalimage,mvp


#endif
