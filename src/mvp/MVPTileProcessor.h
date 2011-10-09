/// \file MVPTileProcessor.h
///
/// MVP Tile Processor class
///
/// TODO: write something here
///

#ifndef __MVPTILEPROCESSOR_H__
#define __MVPTILEPROCESSOR_H__

#include "MVPJobRequest.pb.h"
#include "OrbitalImage.h"

#include <vw/Plate/PlateGeoReference.h>

namespace mvp {

class MVPTileProcessor {
  vw::cartography::GeoReference m_georef;
  int m_tile_size;
  vw::Vector2 post_height_limits;
  MVPAlgorithm m_algorithm;

  // Store cameras and images as octave types too
  std::vector<vw::camera::PinholeModel> m_cameras;
  std::vector<vw::ImageView<vw::float32> > m_images;
  // TODO: Include seeds

  public:
    struct result_type {
      ImageView<vw::float32> posts;
      ImageView<vw::float32> variance;

      ImageView<vw::Vector3f> orientation;
      ImageView<vw::Vector3f> windows;

      result_type(int tile_size) : 
        posts(tile_size), variance(tile_size), orientation(tile_size), windows(tile_size) {}
    };

    MVPTileProcessor(vw::cartography::GeoReference georef, int tile_size, MVPAlgorithm algorithm, 
                     std::vector<vw::camera::PinholeModel> cameras, std::vector<vw::ImageView<vw::float32> > images) :
      m_georef(georef), m_tile_size(tile_size), m_algorithm(algorithm), m_images(images), m_cameras(cameras) {}

    MVPTileProcessor(MVPJobRequest request) {

      // TODO: Actually crop the images and camera files!

    }

    /* // Load from problem file
    MVPTileProcessor(std::string prob_filename)

    // Write probfile
    void write_prob(std::string prob_filename)
    */

    MVPTileResult process() {
      // TODO: some sort of seed propigation scheme...


    }
};

} // namespace mvp

#endif
