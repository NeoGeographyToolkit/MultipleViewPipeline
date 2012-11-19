/// \file ImageCropCollection.h
///
/// Image Crop Collection class
///
/// TODO: Write something here
///

#include <mvp/Image/OrbitalImageDesc.pb.h>
#include <mvp/Image/ImageCrop.h>

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

    void push_back(image::OrbitalImageDesc const& image_file) {
      push_back(image_file.image_path(), image_file.camera_path());
    }

    void push_back(std::string const& image_path, std::string const& camera_path);
};

}} // namespace image,mvp
