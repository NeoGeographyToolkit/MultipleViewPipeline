/// \file OrbitalImageCrop.h
///
/// Orbital Image Crop class
///
/// TODO: Write something here

#ifndef __MVP_ORBITALIMAGECROP_H__
#define __MVP_ORBITALIMAGECROP_H__

#include <mvp/OrbitalImageFileDescriptor.pb.h>

#include <vw/Image/ImageView.h>
#include <vw/Image/MaskViews.h>
#include <vw/FileIO/DiskImageView.h>
#include <vw/Camera/PinholeModel.h>

#include <boost/foreach.hpp>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <vw/Octave/Conversions.h>
#endif

namespace mvp {

vw::camera::PinholeModel offset_pinhole(vw::camera::PinholeModel const& cam, vw::Vector2 const& offset) {
  vw::camera::PinholeModel result(cam);
  result.set_point_offset(result.point_offset() - offset);
  return result;
}

class OrbitalImageCrop {
  vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > m_image;
  vw::camera::PinholeModel m_camera;

  public:
    OrbitalImageCrop(OrbitalImageFileDescriptor const& image_file, vw::BBox2 const& lonlat_bbox) {
      // TODO: Create crops
      try {
        m_image = vw::DiskImageView<vw::PixelMask<vw::PixelGray<vw::float32> > >(image_file.image_path());
      } catch (vw::ArgumentErr& ex) {
        // TODO: Require orbital images to have alpha channels... creating a mask is slow!
        m_image = create_mask(vw::DiskImageView<vw::PixelGray<vw::float32> >(image_file.image_path()), std::numeric_limits<vw::float32>::quiet_NaN());
      }
      m_camera = vw::camera::PinholeModel(image_file.camera_path());
    }

    vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > image() const {return m_image;}
    vw::camera::PinholeModel camera() const {return m_camera;}
};

class OrbitalImageCropCollection : public std::vector<OrbitalImageCrop> {

  vw::BBox2 m_lonlat_bbox;

  public:
    
    OrbitalImageCropCollection(vw::BBox2 const& lonlat_bbox) : m_lonlat_bbox(lonlat_bbox) {}

    void add_image(OrbitalImageFileDescriptor const& image) {
      push_back(OrbitalImageCrop(image, m_lonlat_bbox));
    }
    
    template <class CollectionT>
    void add_image_collection(CollectionT const& orbital_images) {
      BOOST_FOREACH(OrbitalImageFileDescriptor const& o, orbital_images) {
        add_image(o);
      }
    }
};

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
