/// \file ImageCrop.h
///
/// Image Crop class
///
/// TODO: Write something here
///

#ifndef __MVP_IMAGE_IMAGECROP_H__
#define __MVP_IMAGE_IMAGECROP_H__

#include <vw/Image/ImageView.h>
#include <vw/Image/PixelTypes.h>
#include <vw/Image/PixelMask.h>

#include <vw/Camera/PinholeModel.h>

#include <vw/Cartography/Datum.h>

#include <vw/Math/Quaternion.h>

// TODO: These functions should be in vw::camera
namespace vw {
namespace camera {
inline PinholeModel crop(PinholeModel const& cam, int32 x, int32 y, int32 width = 0, int32 height = 0) {
  PinholeModel result(cam);
  result.set_point_offset(result.point_offset() - Vector2(x, y));
  return result;
}
inline PinholeModel crop(PinholeModel const& cam, BBox2i const& bbox) {
  return crop(cam, bbox.min().x(), bbox.min().y(), bbox.width(), bbox.height());
}
}} // namespace camera,vw

namespace mvp {
namespace image {

typedef vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > ImageData;

class ImageCrop : public ImageData {
  vw::camera::PinholeModel m_camera;

  public:
    ImageCrop() : ImageData(), m_camera() {}

    template <class ViewT>
    ImageCrop(vw::ImageViewBase<ViewT> const& image, vw::camera::PinholeModel camera) : 
      ImageData(image.impl()), m_camera(camera) {}

    static vw::BBox2 find_crop_bbox(vw::camera::PinholeModel const& camera, 
                                    vw::BBox2 const& lonlat_bbox,
                                    vw::cartography::Datum const& datum, 
                                    vw::Vector2 const& alt_limits);

    static ImageCrop construct_from_paths(std::string const& image_path,
                                          std::string const& camera_path);

    static ImageCrop construct_from_paths(std::string const& image_path,
                                          std::string const& camera_path,
                                          vw::BBox2 const& lonlat_bbox,
                                          vw::cartography::Datum const& datum,
                                          vw::Vector2 const& alt_limits);

    vw::camera::PinholeModel camera() const {return m_camera;}

    ImageData project(vw::Vector3 const& xyz, 
                      vw::Quat const& orientation, 
                      vw::Vector2 const& scale,
                      vw::Vector2i const& size);

};

}} // namespace image,mvp

#endif
