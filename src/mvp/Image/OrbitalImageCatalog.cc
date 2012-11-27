#include <mvp/Image/OrbitalImageCatalog.h>

namespace mvp {
namespace image {

OrbitalImageCatalog::CatalogEntry::CatalogEntry(std::string const& i, std::string const& c) :
  image_path(i), camera_path(c) {}

void OrbitalImageCatalog::add_image(std::string const& image_path, std::string const& camera_path) {

}

void OrbitalImageCatalog::add_image_pattern(std::string const& image_pattern, std::string const& camera_pattern) {

}

ConvexPolygon OrbitalImageCatalog::find_image_roi(ConvexPolygon const& map_roi,
                                                  vw::camera::CameraModel const& camera,
                                                  vw::cartography::Datum const& datum,
                                                  vw::Vector2 const& alt_limits) {
  return ConvexPolygon();
}

std::vector<OrbitalImageDesc> OrbitalImageCatalog::images_in_region(ConvexPolygon const& map_roi) {

  return std::vector<OrbitalImageDesc>();
}

}} // namespace image,mvp
