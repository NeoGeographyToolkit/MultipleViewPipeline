#include <mvp/OrbitalImageCrop.h>

#include <vw/FileIO/DiskImageView.h>
#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/Image/Algorithms.h>

static vw::ImageViewRef<vw::PixelMask<vw::PixelGray<vw::float32> > > rsrc_helper(boost::shared_ptr<vw::DiskImageResource> rsrc) {
  switch(rsrc->format().pixel_format) {
    case vw::VW_PIXEL_GRAYA:
      return vw::DiskImageView<vw::PixelMask<vw::PixelGray<vw::float32> > >(rsrc);
      break;
    case vw::VW_PIXEL_GRAY:
      return vw::pixel_cast<vw::PixelMask<vw::PixelGray<vw::float32> > >(vw::DiskImageView<vw::PixelGray<vw::float32> >(rsrc));
      break;
    default:
      vw::vw_throw(vw::ArgumentErr() << "Unsupported orbital image pixel format: " << vw::pixel_format_name(rsrc->format().pixel_format));
  }
}

namespace mvp {

vw::BBox2 OrbitalImageCrop::find_crop_bbox(vw::camera::PinholeModel const& camera, 
                                           vw::BBox2 const& lonlat_bbox,
                                           vw::cartography::Datum const& datum, 
                                           vw::Vector2 const& alt_limits) 
{
  VW_ASSERT(datum.semi_major_axis() == datum.semi_minor_axis(), vw::LogicErr() << "Spheroid datums not supported");
  vw::Vector2 radius_range(alt_limits + vw::Vector2(datum.semi_major_axis(), datum.semi_major_axis()));

  std::vector<vw::Vector3> llr_bound_pts(8);
  llr_bound_pts[0] = vw::Vector3(lonlat_bbox.min()[0], lonlat_bbox.min()[1], radius_range[0]);
  llr_bound_pts[1] = vw::Vector3(lonlat_bbox.min()[0], lonlat_bbox.max()[1], radius_range[0]);
  llr_bound_pts[2] = vw::Vector3(lonlat_bbox.max()[0], lonlat_bbox.max()[1], radius_range[0]);
  llr_bound_pts[3] = vw::Vector3(lonlat_bbox.max()[0], lonlat_bbox.min()[1], radius_range[0]);
  llr_bound_pts[4] = vw::Vector3(lonlat_bbox.min()[0], lonlat_bbox.min()[1], radius_range[1]);
  llr_bound_pts[5] = vw::Vector3(lonlat_bbox.min()[0], lonlat_bbox.max()[1], radius_range[1]);
  llr_bound_pts[6] = vw::Vector3(lonlat_bbox.max()[0], lonlat_bbox.max()[1], radius_range[1]);
  llr_bound_pts[7] = vw::Vector3(lonlat_bbox.max()[0], lonlat_bbox.min()[1], radius_range[1]);

  vw::BBox2 cropbox;
  BOOST_FOREACH(vw::Vector3 const& llr, llr_bound_pts) {
    vw::Vector3 xyz(vw::cartography::lon_lat_radius_to_xyz(llr));
    cropbox.grow(camera.point_to_pixel(xyz));
  }

  return cropbox;
}

OrbitalImageCrop OrbitalImageCrop::construct_from_paths(std::string const& image_path,
                                                        std::string const& camera_path) 
{
  boost::shared_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
  return OrbitalImageCrop(rsrc_helper(rsrc), vw::camera::PinholeModel(camera_path));
}

OrbitalImageCrop OrbitalImageCrop::construct_from_paths(std::string const& image_path,
                                                        std::string const& camera_path,
                                                        vw::BBox2 const& lonlat_bbox,
                                                        vw::cartography::Datum const& datum,
                                                        vw::Vector2 const& alt_limits) 
{
  boost::shared_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_path));
  vw::camera::PinholeModel camera(camera_path);

  vw::BBox2 cropbox(find_crop_bbox(camera, lonlat_bbox, datum, alt_limits));
  cropbox.crop(vw::BBox2i(0, 0, rsrc->cols(), rsrc->rows()));

  // Return empty if smaller than a pixel
  if (cropbox.width() < 1 || cropbox.height() < 1) {
    return OrbitalImageCrop();
  }

  cropbox = vw::grow_bbox_to_int(cropbox);

  return OrbitalImageCrop(vw::crop(rsrc_helper(rsrc), cropbox), vw::camera::crop(camera, cropbox));
}


} // namespace mvp
