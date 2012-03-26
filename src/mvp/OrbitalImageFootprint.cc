#include <mvp/OrbitalImageFootprint.h>

#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/FileIO/DiskImageResource.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>


namespace mvp {

vw::Vector2 OrbitalImageFootprint::backproj_px(vw::camera::PinholeModel const& cam, 
                                               vw::Vector2 const& px, 
                                               vw::cartography::Datum const& datum, 
                                               double alt) 
{
  VW_ASSERT(datum.semi_major_axis() == datum.semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 

  double sphere_rad = datum.semi_major_axis() + alt;

  vw::Vector3 dir = cam.pixel_to_vector(px);
  vw::Vector3 cntr = cam.camera_center();

  double a = dot_prod(dir, dir);
  double b = 2 * dot_prod(cntr, dir);
  double c = dot_prod(cntr, cntr) - sphere_rad * sphere_rad;

  double t = (-b - sqrt(b*b-4*a*c))/2/a;

  vw::Vector3 isect = cntr + t * dir;

  vw::Vector3 llr = vw::cartography::xyz_to_lon_lat_radius(isect);

  return vw::math::subvector(llr, 0, 2);
}

OrbitalImageFootprint OrbitalImageFootprint::construct_from_paths(std::string const& image_path, 
                                                                  std::string const& camera_path, 
                                                                  vw::cartography::Datum const& datum, 
                                                                  vw::Vector2 const& alt_limits)
{
  OrbitalImageFileDescriptor image_file;
  image_file.set_camera_path(camera_path);
  image_file.set_image_path(image_path);

  boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_file.image_path()));
  vw::Vector2i image_size(rsrc->cols(), rsrc->rows());

  std::vector<vw::Vector2> fp_points(8);

  vw::camera::PinholeModel camera(image_file.camera_path());
  fp_points[0] = backproj_px(camera, vw::Vector2i(0, 0), datum, alt_limits[0]);
  fp_points[1] = backproj_px(camera, vw::Vector2i(image_size.x(), 0), datum, alt_limits[0]);
  fp_points[2] = backproj_px(camera, vw::Vector2i(image_size.x(), image_size.y()), datum, alt_limits[0]);
  fp_points[3] = backproj_px(camera, vw::Vector2i(0, image_size.y()), datum, alt_limits[0]);
  fp_points[4] = backproj_px(camera, vw::Vector2i(0, 0), datum, alt_limits[1]);
  fp_points[5] = backproj_px(camera, vw::Vector2i(image_size.x(), 0), datum, alt_limits[1]);
  fp_points[6] = backproj_px(camera, vw::Vector2i(image_size.x(), image_size.y()), datum, alt_limits[1]);
  fp_points[7] = backproj_px(camera, vw::Vector2i(0, image_size.y()), datum, alt_limits[1]);

  // TODO: Need to take care of wrapping around poles!
  vw::BBox2 maxbounds(-180,-180,360,360);
  BOOST_FOREACH(vw::Vector2 v, fp_points) {
    if (!maxbounds.contains(v)) {
      VW_ASSERT(maxbounds.contains(v), vw::NoImplErr() << "Lonlat wrapping in footprints needs to be implemented");
    }
  }

  return OrbitalImageFootprint(image_file, image_size, fp_points);
}

int OrbitalImageFootprint::equal_resolution_level() const {
  vw::BBox2 fp_bbox(bounding_box());

  double x_res_lvl = log(360.0 / fp_bbox.width()) / log(2);
  double y_res_lvl = log(360.0 / fp_bbox.height()) / log(2);

  return round(std::max(x_res_lvl, y_res_lvl));
}

int OrbitalImageFootprint::equal_density_level(int tile_size) const {
  vw::BBox2 fp_bbox(bounding_box());

  double x_dens_lvl = log(360.0 * m_image_size.x() / fp_bbox.width() / tile_size) / log(2);
  double y_dens_lvl = log(360.0 * m_image_size.y() / fp_bbox.height() / tile_size) / log(2);

  return ceil(std::max(x_dens_lvl, y_dens_lvl));
}


} // namespace mvp
