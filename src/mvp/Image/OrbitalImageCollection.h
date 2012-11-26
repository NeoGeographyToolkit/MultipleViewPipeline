/// \file ImageCropCollection.h
///
/// Image Crop Collection class
///
/// TODO: Write something here
///

#ifndef __MVP_IMAGE_ORBITALIMAGECOLLECTION_H__
#define __MVP_IMAGE_ORBITALIMAGECOLLECTION_H__

#include <mvp/Image/OrbitalImage.h>

#include <vw/Image/AlgorithmFunctions.h>

namespace mvp {
namespace image {

class OrbitalImageCollection : public std::vector<OrbitalImage> {

  public:

    std::vector<vw::ImageView<OrbitalImagePixel> > back_project(vw::Vector3 const& xyz, 
                                                          vw::Quat const& orientation, 
                                                          vw::Vector2 const& scale,
                                                          vw::Vector2i const& size);

};

}} // namespace image,mvp

#endif
