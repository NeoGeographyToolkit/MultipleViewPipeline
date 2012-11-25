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

namespace mvp {
namespace image {

typedef vw::PixelMask<vw::PixelGray<vw::float32> > OrbitalData;

class OrbitalImage {
  vw::ImageViewRef<OrbitalData> m_image;
  boost::shared_ptr<vw::camera::CameraModel> m_camera;

  public:
    template <class ImageT>
    OrbitalImage(vw::ImageViewBase<ImageT> image, boost::shared_ptr<vw::camera::CameraModel> camera) :
      m_image(image.impl()), m_camera(camera) {}

    template <class ImageT, class CameraT>
    OrbitalImage(vw::ImageViewBase<ImageT> image, CameraT camera) :
      m_image(image.impl()), m_camera(new CameraT(camera)) {}

    OrbitalImage(OrbitalImageDesc const& desc);

    vw::ImageViewRef<OrbitalData> image() const {return m_image;}

    boost::shared_ptr<vw::camera::CameraModel> camera() const {return m_camera;}

    static OrbitalImageDesc create_desc(std::string const& image_path,
                                        std::string const& camera_path, 
                                        vw::BBox2 const& lonlat_bbox,
                                        vw::cartography::Datum const& datum,
                                        vw::Vector2 const& alt_limits); 

    // Only can write PinholeModels for now...
    OrbitalImageDesc write(std::string const& prefix);

    ImageView<OrbitalData> project(vw::Vector3 const& xyz, 
                                   vw::Quat const& orientation, 
                                   vw::Vector2 const& scale,
                                   vw::Vector2i const& size);
};

}} // namespace image,mvp

#endif
