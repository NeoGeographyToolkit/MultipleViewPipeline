/// \file OrbitalImageCrop.h
///
/// Orbital Image Crop class
///
/// TODO: Write something here

#ifndef __MVP_ORBITALIMAGECROP_H__
#define __MVP_ORBITALIMAGECROP_H__

#include <vw/Image/ImageView.h>
#include <vw/Camera/PinholeModel.h>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <vw/Octave/Conversions.h>
#endif

namespace mvp {

class OrbitalImageCrop {
  vw::camera::PinholeModel m_camera;
  vw::ImageView<double> m_image;

  public:
    OrbitalImageCrop(OrbitalImageFileDesc const& desc, vw::BBox2 const& lonlat_bbox) :
      m_camera(desc.camera_path()), m_image(desc.image_path()) {
      // TODO: Create crops instead
    }

    vw::camera::PinholeModel camera() const {return m_camera;}
    vw::ImageView<double> image() const {return m_image;}
};

typedef std::vector<OrbitalImageCrop> OrbitalImageCropCollection;

#if MVP_ENABLE_OCTAVE_SUPPORT
class OrbitalImageCropOctave {
  ::Matrix m_camera;
  ::Matrix m_image;

  public:
    OrbitalImageCropOctave(OrbitalImageCrop const& oic) :
      m_camera(vw::octave::pinhole_to_octave(oic.camera())),
      m_image(vw::octave::imageview_to_octave(oic.image())) {}

    ::Matrix camera() const {return m_camera;}
    ::Matrix image() const {return m_image;}

    // octave_value_list struct
}
#endif

} // namespace mvp

#endif
