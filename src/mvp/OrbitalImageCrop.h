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
#include <vw/Cartography/Datum.h>

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
    static OrbitalImageCrop construct_from_descriptor(OrbitalImageFileDescriptor const& image_file, 
                                                      vw::BBox2 const& lonlat_bbox, 
                                                      vw::cartography::Datum const& datum,
                                                      vw::Vector2 const& post_height_limits) {
      vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > image;

      // TODO: Create crops
      try {
        image = vw::DiskImageView<vw::PixelMask<vw::PixelGray<vw::float32> > >(image_file.image_path());
      } catch (vw::ArgumentErr& ex) {
        // TODO: Require orbital images to have alpha channels... creating a mask is slow!
        image = create_mask(vw::DiskImageView<vw::PixelGray<vw::float32> >(image_file.image_path()), std::numeric_limits<vw::float32>::quiet_NaN());
      }
      
      vw::camera::PinholeModel camera(image_file.camera_path());

      return OrbitalImageCrop(image, camera);
    }

    vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > image() const {return m_image;}
    vw::camera::PinholeModel camera() const {return m_camera;}

  protected:
    // Make sure the user doesn't construct one
    template <class ViewT>
    OrbitalImageCrop(vw::ImageViewBase<ViewT> const& image, vw::camera::PinholeModel camera) : m_image(image.impl()), m_camera(camera) {}
};

class OrbitalImageCropCollection : public std::vector<OrbitalImageCrop> {

  vw::BBox2 m_lonlat_bbox;
  vw::cartography::Datum m_datum;
  vw::Vector2 m_post_height_limits;

  public:
    
    OrbitalImageCropCollection(vw::BBox2 const& lonlat_bbox, vw::cartography::Datum const& datum, vw::Vector2 const& post_height_limits) : 
      m_lonlat_bbox(lonlat_bbox), m_datum(datum), m_post_height_limits(post_height_limits) {}

    void add_image(OrbitalImageFileDescriptor const& image) {
      push_back(OrbitalImageCrop::construct_from_descriptor(image, m_lonlat_bbox, m_datum, m_post_height_limits));
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
