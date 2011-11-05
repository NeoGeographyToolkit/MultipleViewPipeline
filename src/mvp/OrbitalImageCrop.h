/// \file OrbitalImageCrop.h
///
/// Orbital Image Crop class
///
/// TODO: Write something here

#ifndef __MVP_ORBITALIMAGECROP_H__
#define __MVP_ORBITALIMAGECROP_H__

#include <mvp/OrbitalImageFileDescriptor.pb.h>

#include <vw/Image/ImageView.h>
#include <vw/Image/ImageViewRef.h>
#include <vw/Image/Algorithms.h>
#include <vw/Image/MaskViews.h>
#include <vw/Image/Transform.h> // grow_bbox_to_int
#include <vw/FileIO/DiskImageView.h>
#include <vw/Camera/PinholeModel.h>
#include <vw/Cartography/Datum.h>
#include <vw/Cartography/SimplePointImageManipulation.h>

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
    template <class ViewT>
    OrbitalImageCrop(vw::ImageViewBase<ViewT> const& image, vw::camera::PinholeModel camera) : m_image(image.impl()), m_camera(camera) {}

    vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > image() const {return m_image;}
    vw::camera::PinholeModel camera() const {return m_camera;}
};

class OrbitalImageCropCollection : public std::vector<OrbitalImageCrop> {

  vw::BBox2 m_lonlat_bbox;
  vw::cartography::Datum m_datum;
  vw::Vector2 m_post_height_limits;

  public:
    
    OrbitalImageCropCollection(vw::BBox2 const& lonlat_bbox, vw::cartography::Datum const& datum, vw::Vector2 const& post_height_limits) : 
      m_lonlat_bbox(lonlat_bbox), m_datum(datum), m_post_height_limits(post_height_limits) {
      VW_ASSERT(m_datum.semi_major_axis() == m_datum.semi_minor_axis(), vw::LogicErr() << "Spheroid datums not supported");
    }

    void add_image(OrbitalImageFileDescriptor const& image_file) {
      boost::scoped_ptr<vw::DiskImageResource> rsrc(vw::DiskImageResource::open(image_file.image_path()));
      vw::BBox2i imagebox(0, 0, rsrc->cols(), rsrc->rows());

      vw::Vector2 radius_range(m_post_height_limits + vw::Vector2(m_datum.semi_major_axis(), m_datum.semi_major_axis()));

      vw::camera::PinholeModel camera(image_file.camera_path());

      std::vector<vw::Vector3> llr_bound_pts(8);
      llr_bound_pts[0] = vw::Vector3(m_lonlat_bbox.min()[0], m_lonlat_bbox.min()[1], radius_range[0]);
      llr_bound_pts[1] = vw::Vector3(m_lonlat_bbox.min()[0], m_lonlat_bbox.max()[1], radius_range[0]);
      llr_bound_pts[2] = vw::Vector3(m_lonlat_bbox.max()[0], m_lonlat_bbox.max()[1], radius_range[0]);
      llr_bound_pts[3] = vw::Vector3(m_lonlat_bbox.max()[0], m_lonlat_bbox.min()[1], radius_range[0]);
      llr_bound_pts[4] = vw::Vector3(m_lonlat_bbox.min()[0], m_lonlat_bbox.min()[1], radius_range[1]);
      llr_bound_pts[5] = vw::Vector3(m_lonlat_bbox.min()[0], m_lonlat_bbox.max()[1], radius_range[1]);
      llr_bound_pts[6] = vw::Vector3(m_lonlat_bbox.max()[0], m_lonlat_bbox.max()[1], radius_range[1]);
      llr_bound_pts[7] = vw::Vector3(m_lonlat_bbox.max()[0], m_lonlat_bbox.min()[1], radius_range[1]);

      vw::BBox2 cropbox;
      BOOST_FOREACH(vw::Vector3 const& llr, llr_bound_pts) {
        vw::Vector3 xyz(vw::cartography::lon_lat_radius_to_xyz(llr));
        cropbox.grow(camera.point_to_pixel(xyz));
      }

      cropbox.crop(imagebox);

      vw::ImageView<vw::PixelMask<vw::PixelGray<vw::float32> > > image;

      if (cropbox.width() > 1e-6 && cropbox.height() > 1e-6) {
        // TODO: Need a better way of determining alpha in VW
        switch(rsrc->format().pixel_format) {
          case vw::VW_PIXEL_GRAYA:
          case vw::VW_PIXEL_SCALAR_MASKED:
          case vw::VW_PIXEL_GRAY_MASKED:
            image = vw::crop(vw::DiskImageView<vw::PixelMask<vw::PixelGray<vw::float32> > >(image_file.image_path()), vw::grow_bbox_to_int(cropbox));
            break;
          default:
            image = vw::crop(create_mask(vw::DiskImageView<vw::PixelGray<vw::float32> >(image_file.image_path()), std::numeric_limits<vw::float32>::quiet_NaN()), 
                             vw::grow_bbox_to_int(cropbox));
        }
        push_back(OrbitalImageCrop(image, offset_pinhole(camera, cropbox.min())));
      }
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
