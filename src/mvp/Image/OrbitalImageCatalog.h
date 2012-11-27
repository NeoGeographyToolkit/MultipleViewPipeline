/// \file OrbitalImageCatalog.h
///
/// Orbital Image class
///
/// TODO: Write something here
///

#include <mvp/Core/ConvexPolygon.h>
#include <mvp/Image/OrbitalImageDesc.pb.h>

#include <vw/Camera/CameraModel.h>
#include <vw/Cartography/Datum.h>

#ifndef __MVP_IMAGE_ORBITALIMAGECATALOG_H__
#define __MVP_IMAGE_ORBITALIMAGECATALOG_H__

namespace mvp {
namespace image {

class OrbitalImageCatalog {
  vw::cartography::Datum m_datum;
  vw::Vector2 m_alt_limits;

  struct CatalogEntry {
    std::string image_path;
    vw::BBox2i image_bbox;
    std::string camera_path;
    boost::shared_ptr<vw::camera::CameraModel> camera;

    CatalogEntry(std::string const& image_path, std::string const& camera_path);
  };

  std::vector<CatalogEntry> m_entries;

  public:
    OrbitalImageCatalog(vw::cartography::Datum const& datum, vw::Vector2 const& alt_limits) :
      m_datum(datum), m_alt_limits(alt_limits), m_entries() {}

    void add_image(std::string const& image_path, std::string const& camera_path);

    void add_image_pattern(std::string const& image_pattern, std::string const& camera_pattern);

    int size() const {return m_entries.size();}

    static ConvexPolygon find_image_roi(ConvexPolygon const& map_roi,
                                        vw::camera::CameraModel const& camera,
                                        vw::cartography::Datum const& datum,
                                        vw::Vector2 const& alt_limits);

    std::vector<OrbitalImageDesc> images_in_region(ConvexPolygon const& lonlat_roi) const;
};

}} // image,mvp

#endif
