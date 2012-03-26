/// \file OrbitalImageCrop.h
///
/// Orbital Image Crop class
///
/// TODO: Write something here

#ifndef __MVP_ORBITALIMAGECROP_H__
#define __MVP_ORBITALIMAGECROP_H__

#include <mvp/Config.h>
#include <mvp/OrbitalImageFileDescriptor.pb.h>

#include <vw/Image/ImageView.h>
#include <vw/Image/ImageViewRef.h>
#include <vw/Image/PixelMask.h>

#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/Datum.h>

#include <boost/foreach.hpp>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <vw/Octave/Conversions.h>
#endif

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
}} // vw::camera

namespace mvp {

class OrbitalImageCrop : public vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > {
  vw::camera::PinholeModel m_camera;

  public:
    static vw::BBox2i find_crop_bbox(vw::camera::PinholeModel const& camera, 
                                     vw::BBox2 const& lonlat_bbox,
                                     vw::cartography::Datum const& datum, 
                                     vw::Vector2 const& alt_limits);

    static OrbitalImageCrop construct_from_paths(std::string const& image_path,
                                                 std::string const& camera_path);

    static OrbitalImageCrop construct_from_paths(std::string const& image_path,
                                                 std::string const& camera_path,
                                                 vw::BBox2 const& lonlat_bbox,
                                                 vw::cartography::Datum const& datum,
                                                 vw::Vector2 const& alt_limits);

    vw::camera::PinholeModel camera() const {return m_camera;}

  protected:
    // Make sure the user doesn't construct one like this
    OrbitalImageCrop() : vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > >(), m_camera() {}

    template <class ViewT>
    OrbitalImageCrop(vw::ImageViewBase<ViewT> const& image, vw::camera::PinholeModel camera) : 
      vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > >(image.impl()), m_camera(camera) {}
};

class OrbitalImageCropCollection : public std::vector<OrbitalImageCrop> {

  vw::BBox2 m_lonlat_bbox;
  vw::cartography::Datum m_datum;
  vw::Vector2 m_alt_limits;

  public:

    // This constructor constructs an OrbitalImageCropCollection that doesn't crop the images  
    OrbitalImageCropCollection() : m_lonlat_bbox(), m_datum(), m_alt_limits() {} 

    OrbitalImageCropCollection(vw::BBox2 const& lonlat_bbox, vw::cartography::Datum const& datum, vw::Vector2 const& alt_limits) : 
      m_lonlat_bbox(lonlat_bbox), m_datum(datum), m_alt_limits(alt_limits) {
      VW_ASSERT(m_datum.semi_major_axis() == m_datum.semi_minor_axis(), vw::LogicErr() << "Spheroid datums not supported");
    }

    OrbitalImageCropCollection(vw::BBox2 const& lonlat_bbox, vw::cartography::Datum const& datum, double alt_limit_min, double alt_limit_max) : 
      m_lonlat_bbox(lonlat_bbox), m_datum(datum), m_alt_limits(alt_limit_min, alt_limit_max) {
      VW_ASSERT(m_datum.semi_major_axis() == m_datum.semi_minor_axis(), vw::LogicErr() << "Spheroid datums not supported");
    }

    void add_image(OrbitalImageFileDescriptor const& image_file) {
      add_image(image_file.image_path(), image_file.camera_path());
    }

    void add_image(std::string const& image_path, std::string const& camera_path) {
      if (m_lonlat_bbox.empty()) {
        // If the lonlat bbox is empty, we don't crop the images
        push_back(OrbitalImageCrop::construct_from_paths(image_path, camera_path));
      } else {
        OrbitalImageCrop image(OrbitalImageCrop::construct_from_paths(image_path, camera_path, m_lonlat_bbox, m_datum, m_alt_limits));
        if (image.cols() > 0 && image.rows() > 0) {
          push_back(image);
        }
      }
    }
    
    template <class CollectionT>
    void add_image_collection(CollectionT const& orbital_images) {
      BOOST_FOREACH(OrbitalImageFileDescriptor const& o, orbital_images) {
        add_image(o);
      }
    }

    #if MVP_ENABLE_OCTAVE_SUPPORT
    ::octave_map to_octave() const {
      ::octave_value_list datas;
      ::octave_value_list cameras;

      BOOST_FOREACH(OrbitalImageCrop const& o, *this) {
        datas.append(vw::octave::imageview_to_octave(o));
        cameras.append(vw::octave::pinhole_to_octave(o.camera()));
      }

      ::octave_map result;
      result.assign("data", datas);
      result.assign("camera", cameras);
      return result;
    }
    #endif
};

} // namespace mvp

#endif
