#include <mvp/Image/ImageCropCollection.h>

namespace mvp {
namespace image {

void ImageCropCollection::push_back(std::string const& image_path, std::string const& camera_path) {
  if (m_lonlat_bbox.empty()) {
    // If the lonlat bbox is empty, we don't crop the images
    std::vector<ImageCrop>::push_back(ImageCrop::construct_from_paths(image_path, camera_path));
  } else {
    ImageCrop image(ImageCrop::construct_from_paths(image_path, camera_path, m_lonlat_bbox, m_datum, m_alt_limits));
    if (image.cols() > 0 && image.rows() > 0) {
      std::vector<ImageCrop>::push_back(image);
    }
  }
}

}} // namespace image,mvp
