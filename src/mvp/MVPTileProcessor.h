/// \file MVPTileProcessor.h
///
/// MVP Tile Processor class
///
/// TODO: write something here
///

#ifndef __MVP_MVPTILEPROCESSOR_H__
#define __MVP_MVPTILEPROCESSOR_H__

#include <mvp/MVPJobRequest.pb.h>

#include <mvp/MVPAlgorithm.h>
#include <mvp/OrbitalImageFile.h>
#include <mvp/OrbitalImageCrop.h>

#include <vw/Plate/PlateGeoReference.h>

namespace mvp {

struct MVPTileResult {
  vw::cartography::GeoReference georef;

  vw::ImageView<vw::float32> post_height;
  vw::ImageView<vw::float32> variance;

  vw::ImageView<vw::Vector3f> orientation;
  vw::ImageView<vw::Vector3f> windows;

  MVPTileResult(vw::cartography::GeoReference g, int tile_size) : georef(g),
    post_height(tile_size, tile_size), variance(tile_size, tile_size), 
    orientation(tile_size, tile_size), windows(tile_size, tile_size) {}

  void update(int col, int row, MVPAlgorithmResult const& px_result) {
    post_height(col, row) = px_result.post_height;
    variance(col, row) = px_result.variance;

    orientation(col, row) = px_result.orientation;
    windows(col, row) = px_result.windows;
  }
};

// TODO: This should be replaced by a function in VW
vw::cartography::GeoReference offset_georef(vw::cartography::GeoReference const& georef, int cols, int rows) {
  vw::Matrix3x3 offset;

  offset.set_identity();
  offset(0, 2) = cols;
  offset(1, 2) = rows;

  vw::cartography::GeoReference result(georef);
  // TODO: This is slow...
  result.set_transform(georef.transform() * offset);
  return result; 
}

class MVPTileProcessor {
  vw::cartography::GeoReference m_georef;
  int m_tile_size;
  OrbitalImageCropCollection m_orbital_images;
  MVPAlgorithm m_algorithm;

  public:

    MVPTileProcessor(MVPJobRequest const& request) {
      int col = request.tile().col(), row = request.tile().row(), level = request.tile().level();
      vw::platefile::PlateGeoReference plate_georef(request.plate_georef());
      m_georef = plate_georef.tile_georef(col, row, level);
      m_tile_size = plate_georef.tile_size();

      BOOST_FOREACH(OrbitalImageFileDesc const& o, request.orbital_images()) {
        m_orbital_images.push_back(OrbitalImageCrop(o, plate_georef.tile_lonlat_bbox(col, row, level)));
      }

      m_algorithm = MVPAlgorithm(request.algorithm_settings(), m_orbital_images); 
    }

    OrbitalImageCropCollection orbital_images() const {
      return m_orbital_images;
    }

    /* // Load from problem file
    MVPTileProcessor(std::string prob_filename)

    // Write probfile
    void write_prob(std::string prob_filename)
    */

    MVPTileResult process(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) {
      MVPTileResult tile_result(m_georef, m_tile_size);
      MVPAlgorithmVar seed;

      int curr_px_num = 0;
      int num_px_to_process = m_tile_size * m_tile_size;
      progress.report_progress(0);
      for (int col = 0; col < m_tile_size; col++) {
        for (int row = 0; row < m_tile_size; row++) {
          MVPAlgorithmResult px_result = m_algorithm(seed, offset_georef(m_georef, col, row));
          tile_result.update(col, row, px_result);
          progress.report_progress(double(++curr_px_num) / num_px_to_process);
        }
      }
      progress.report_finished();
      return tile_result;
    }
};

} // namespace mvp

#endif
