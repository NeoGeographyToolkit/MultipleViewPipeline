#include <mvp/Image/OrbitalImageCatalog.h>

#include <vw/FileIO/DiskImageResource.h>
#include <vw/Camera/PinholeModel.h>

#include <boost/foreach.hpp>

namespace mvp {
namespace image {

OrbitalImageCatalog::CatalogEntry::CatalogEntry(std::string const& i, std::string const& c) :
  image_path(i), camera_path(c) 
{
  // TODO: camera loader (in core/CameraHelpers.h)?
  camera.reset(new vw::camera::PinholeModel(camera_path));

  // TODO: image size checker (in core/ImageHelpers.h)?
  boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
  image_bbox = vw::BBox2i(0, 0, rsrc->cols(), rsrc->rows());
}

void OrbitalImageCatalog::add_image(std::string const& image_path, std::string const& camera_path) {
  m_entries.push_back(CatalogEntry(image_path, camera_path));
}

void OrbitalImageCatalog::add_image_pattern(std::string const& image_pattern, std::string const& camera_pattern) {
  std::cout << "add_image_pattern!" << std::endl;
}

ConvexPolygon OrbitalImageCatalog::find_image_roi(ConvexPolygon const& map_roi,
                                                  vw::camera::CameraModel const& camera,
                                                  vw::cartography::Datum const& datum,
                                                  vw::Vector2 const& alt_limits) {
  // TODO: Cull out points on the other side of the globe that should be occluded
  std::vector<vw::Vector2> result;
  BOOST_FOREACH(vw::Vector2 const& v, map_roi.vertices()) {
    result.push_back(camera.point_to_pixel(datum.geodetic_to_cartesian(vw::Vector3(v.x(), v.y(), alt_limits[0]))));
    result.push_back(camera.point_to_pixel(datum.geodetic_to_cartesian(vw::Vector3(v.x(), v.y(), alt_limits[1]))));
  }
  return ConvexPolygon(result);
}

std::vector<OrbitalImageDesc> OrbitalImageCatalog::images_in_region(ConvexPolygon const& map_roi) const {
  std::vector<OrbitalImageDesc> result;
  BOOST_FOREACH(CatalogEntry const& e, m_entries) {
    ConvexPolygon image_roi = find_image_roi(map_roi, *e.camera, m_datum, m_alt_limits);

    if (image_roi.intersects(ConvexPolygon(e.image_bbox))) {
      vw::BBox2i cropbox = vw::grow_bbox_to_int(image_roi.bounding_box());
      cropbox.crop(e.image_bbox);

      if (!cropbox.empty()) {
        OrbitalImageDesc i;
        i.set_image_path(e.image_path);
        i.set_camera_path(e.camera_path);
        i.set_minx(cropbox.min().x());
        i.set_miny(cropbox.min().y());
        i.set_width(cropbox.width());
        i.set_height(cropbox.height());
        result.push_back(i);
      }
    }
  }

  return result;
}

}} // namespace image,mvp
