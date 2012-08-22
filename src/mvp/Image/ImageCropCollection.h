/// \file ImageCropCollection.h
///
/// Image Crop Collection class
///
/// TODO: Write something here
///

#include <mvp/Pipeline/OrbitalImage.pb.h>
#include <mvp/Image/ImageCrop.h>

#include <boost/foreach.hpp>

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <vw/Octave/Conversions.h>
#endif

namespace mvp {
namespace image {

class ImageCropCollection : public std::vector<ImageCrop> {

  vw::BBox2 m_lonlat_bbox;
  vw::cartography::Datum m_datum;
  vw::Vector2 m_alt_limits;

  public:

    // This constructor constructs an ImageCropCollection that doesn't crop the images  
    ImageCropCollection() : m_lonlat_bbox(), m_datum(), m_alt_limits() {} 

    ImageCropCollection(vw::BBox2 const& lonlat_bbox, vw::cartography::Datum const& datum, vw::Vector2 const& alt_limits) : 
      m_lonlat_bbox(lonlat_bbox), m_datum(datum), m_alt_limits(alt_limits) {
      VW_ASSERT(m_datum.semi_major_axis() == m_datum.semi_minor_axis(), vw::LogicErr() << "Spheroid datums not supported");
    }

    void add_image(pipeline::OrbitalImage const& image_file) {
      add_image(image_file.image_path(), image_file.camera_path());
    }

    void add_image(std::string const& image_path, std::string const& camera_path) {
      if (m_lonlat_bbox.empty()) {
        // If the lonlat bbox is empty, we don't crop the images
        push_back(ImageCrop::construct_from_paths(image_path, camera_path));
      } else {
        ImageCrop image(ImageCrop::construct_from_paths(image_path, camera_path, m_lonlat_bbox, m_datum, m_alt_limits));
        if (image.cols() > 0 && image.rows() > 0) {
          push_back(image);
        }
      }
    }
    
    template <class CollectionT>
    void add_image_collection(CollectionT const& orbital_images) {
      BOOST_FOREACH(pipeline::OrbitalImage const& o, orbital_images) {
        add_image(o);
      }
    }

    #if MVP_ENABLE_OCTAVE_SUPPORT
    ::octave_map to_octave() const {
      ::octave_value_list datas;
      ::octave_value_list cameras;

      BOOST_FOREACH(ImageCrop const& o, *this) {
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

}} // namespace image,mvp
