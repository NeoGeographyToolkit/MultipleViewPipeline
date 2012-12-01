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

#include <boost/foreach.hpp>

namespace mvp {
namespace image {

class OrbitalImageCollection : public std::vector<OrbitalImage> {

  public:
    OrbitalImageCollection() {}

    template <class ContainerT>
    OrbitalImageCollection(ContainerT const& orbital_image_descs) {
      BOOST_FOREACH(OrbitalImageDesc const& desc, orbital_image_descs) {
        push_back(OrbitalImage(desc));
      }
    }

    std::vector<vw::ImageView<OrbitalImagePixel> > back_project(vw::Vector3 const& xyz, 
                                                                vw::Quat const& orientation, 
                                                                vw::Vector2 const& scale,
                                                                vw::Vector2i const& size);

};

}} // namespace image,mvp

#endif
