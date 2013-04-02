#include <mvp/Image/OrbitalImageCatalog.h>

#include <mvp/Core/Settings.h>

#include <vw/FileIO/DiskImageResource.h>
#include <vw/Camera/PinholeModel.h>

#include <boost/foreach.hpp>
#include <boost/filesystem.hpp>
#include <boost/regex.hpp>

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

static std::vector<std::string> paths_from_pattern(boost::filesystem::path pattern) {
  namespace fs = boost::filesystem;

  fs::path dir(pattern.parent_path());
  boost::regex filter(pattern.filename().string());

  std::vector<std::string> result_paths;

  fs::directory_iterator end_itr;
  for(fs::directory_iterator i(dir); i != end_itr; i++) {
    if (!fs::is_regular_file(i->status())) {
      continue;
    }

    boost::smatch what;
    if (boost::regex_match(i->path().filename().string(), what, filter)) {
      result_paths.push_back(i->path().string());
    }  
  }

  sort(result_paths.begin(), result_paths.end());

  return result_paths;
}

void OrbitalImageCatalog::add_image_pattern(std::string const& image_pattern, std::string const& camera_pattern) {
  std::vector<std::string> image_paths = paths_from_pattern(image_pattern);
  std::vector<std::string> camera_paths = paths_from_pattern(camera_pattern);

  if (image_paths.size() != camera_paths.size()) {
    vw::vw_throw(vw::InputErr() << "The number of orbital images does not match the number of cameras");
  }

  for (unsigned i = 0; i < image_paths.size(); i++) {
    add_image(image_paths[i], camera_paths[i]);
  }
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
        if (mvp_settings().render().no_orbital_image_cropping()) {
          i.set_minx(0);
          i.set_miny(0);
          i.set_width(e.image_bbox.width());
          i.set_height(e.image_bbox.height());
        } else {
          i.set_minx(cropbox.min().x());
          i.set_miny(cropbox.min().y());
          i.set_width(cropbox.width());
          i.set_height(cropbox.height());
        }
        result.push_back(i);
      }
    }
  }

  return result;
}

}} // namespace image,mvp
