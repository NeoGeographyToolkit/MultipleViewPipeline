/// \file ImageCrop.h
///
/// Image Crop class
///
/// TODO: Write something here
///

#ifndef __MVP_IMAGE_ORBITALIMAGE_H__
#define __MVP_IMAGE_ORBITALIMAGE_H__

#include <mvp/Image/OrbitalImageDesc.pb.h>

#include <vw/Image/ImageView.h>
#include <vw/Image/PixelTypes.h>
#include <vw/Image/PixelMask.h>

#include <vw/Camera/CameraModel.h>

#include <vw/Math/Quaternion.h>

namespace mvp {
namespace image {

typedef vw::PixelMask<vw::PixelGray<vw::float32> > OrbitalImagePixel;

class OrbitalImage {
  vw::ImageView<OrbitalImagePixel> m_image;
  vw::camera::CroppedCamera m_camera;

  public:
    OrbitalImage() : m_image(), m_camera() {}

    template <class ImageT, class CameraT>
    OrbitalImage(vw::ImageViewBase<ImageT> const& image, CameraT camera) :
      m_image(image.impl()), m_camera(camera, vw::Vector2()) {}

    OrbitalImage(OrbitalImageDesc const& desc);

    vw::ImageView<OrbitalImagePixel> image() const {return m_image;}

    vw::camera::CroppedCamera camera() const {return m_camera;}

    // Only can write PinholeModels for now...
    OrbitalImageDesc write(std::string const& prefix) const;

    vw::ImageView<OrbitalImagePixel> back_project(vw::Vector3 const& xyz, 
                                            vw::Quat const& orientation, 
                                            vw::Vector2 const& scale,
                                            vw::Vector2i const& size) const;
};

}} // namespace image,mvp

#endif
