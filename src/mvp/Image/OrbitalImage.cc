#include <mvp/Image/OrbitalImage.h>

#include <vw/FileIO/DiskImageView.h>
#include <vw/Image/Algorithms.h>
#include <vw/Image/ImageViewRef.h>
#include <vw/Image/Interpolation.h>

#include <vw/Camera/PinholeModel.h>

#include <boost/foreach.hpp>

namespace mvp {
namespace image {

static vw::ImageViewRef<OrbitalImagePixel> rsrc_helper(boost::shared_ptr<vw::DiskImageResource> rsrc) {
  switch(rsrc->format().pixel_format) {
    case vw::VW_PIXEL_GRAYA:
      return vw::DiskImageView<OrbitalImagePixel>(rsrc);
      break;
    case vw::VW_PIXEL_GRAY:
      return vw::pixel_cast<OrbitalImagePixel>(vw::DiskImageView<vw::PixelGray<vw::float32> >(rsrc));
      break;
    default:
      vw::vw_throw(vw::ArgumentErr() << "Unsupported orbital image pixel format: " << vw::pixel_format_name(rsrc->format().pixel_format));
  }
}

OrbitalImage::OrbitalImage(OrbitalImageDesc const& desc) {
  vw::BBox2i cropbox(desc.minx(), desc.miny(), desc.width(), desc.height());

  boost::shared_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(desc.image_path()));
  m_image = vw::crop(rsrc_helper(rsrc), cropbox);

  m_camera = vw::camera::crop(vw::camera::PinholeModel(desc.camera_path()), cropbox);
}

// TODO: Only can write PinholeModels for now...
OrbitalImageDesc write(std::string const& prefix) {
  vw::vw_throw(vw::NoImplErr());
  return OrbitalImageDesc();
}

vw::ImageView<OrbitalImagePixel> OrbitalImage::back_project(vw::Vector3 const& xyz, 
                                                      vw::Quat const& orientation,
                                                      vw::Vector2 const& scale,
                                                      vw::Vector2i const& size) const
{
  using namespace vw;

  vw::ImageView<OrbitalImagePixel> patch(size.x(), size.y());
  Matrix3x3 morientation = orientation.rotation_matrix();

  for (int row = 0; row < size.y(); row++) {
    for (int col = 0; col < size.x(); col++) {
      Vector2 patch_pt = elem_prod(Vector2(col, row) - Vector2(size) / 2, scale);
      Vector2 image_pt = m_camera.point_to_pixel(morientation * Vector3(patch_pt.x(), patch_pt.y(), 0) + xyz);

      patch(col, row) = vw::interpolate(m_image, BilinearInterpolation(), ZeroEdgeExtension())(image_pt.x(), image_pt.y());
    }
  }

  return patch;
}

}} // namespace image,mvp
