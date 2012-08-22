/// \file FootprintCollection.h
///
/// A collection to hold footprints
///
/// TODO: Write me!
///

#ifndef __MVP_ORBITALIMAGE_FOOTPRINTCOLLECTION_H__
#define __MVP_ORBITALIMAGE_FOOTPRINTCOLLECTION_H__

#include <mvp/OrbitalImage/Footprint.h>
#include <vw/Cartography/Datum.h>

namespace mvp {
namespace orbitalimage {

class FootprintCollection : public std::vector<Footprint> {
  vw::cartography::Datum m_datum;
  vw::Vector2 m_alt_limits;

  public:
    FootprintCollection(vw::cartography::Datum const& datum, vw::Vector2 const& alt_limits) :
      m_datum(datum), m_alt_limits(alt_limits) {}

    FootprintCollection(vw::cartography::Datum const& datum, double alt_limit_min, double alt_limit_max) :
     m_datum(datum), m_alt_limits(alt_limit_min, alt_limit_max) {}

    void push_back(std::string const& image_path, std::string const& camera_path) {
      std::vector<Footprint>::push_back(Footprint::construct_from_paths(image_path, camera_path, m_datum, m_alt_limits));
    }

    void push_back_pattern(std::string const& image_pattern, std::string const& camera_pattern);

    int equal_resolution_level() const;

    int equal_density_level(int tile_size) const;

    vw::BBox2 lonlat_bbox() const;

    std::vector<OrbitalImage> images_in_region(vw::BBox2 const& lonlat_bbox) const;
};

}} // namespace orbitalimage,mvp

#endif
