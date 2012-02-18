/// \file MVPJobBase.h
///
/// MVP Job Base class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOBBASE_H__
#define __MVP_MVPJOBBASE_H__

#include <vw/Plate/PlateGeoReference.h>
#include <vw/Cartography/GeoReference.h>
#include <vw/Cartography/SimplePointImageManipulation.h>
#include <vw/Image/PixelMask.h>

#include <mvp/Config.h>
#include <mvp/MVPJobRequest.pb.h>
#include <mvp/MVPAlgorithmOptions.pb.h>
#include <mvp/OrbitalImageCrop.h>

#include <boost/foreach.hpp>

namespace mvp {

struct MVPAlgorithmVar {
  vw::float32 alt;

  vw::Vector3f orientation;
  vw::Vector3f windows;

  MVPAlgorithmVar(vw::float32 a = 0, vw::Vector3f const& o = vw::Vector3f(), vw::Vector3f const& w = vw::Vector3f()) :
    alt(a), orientation(o), windows(w) {}

  #if MVP_ENABLE_OCTAVE_SUPPORT
  MVPAlgorithmVar(::octave_scalar_map const& oct_map) : 
    alt(oct_map.getfield("alt").float_value()),
    orientation(vw::octave::octave_to_vector(oct_map.getfield("orientation").matrix_value())),
    windows(vw::octave::octave_to_vector(oct_map.getfield("windows").matrix_value())) {}

  ::octave_scalar_map to_octave() const {
    ::octave_scalar_map result;
    result.setfield("alt", double(alt));
    result.setfield("orientation", vw::octave::vector_to_octave(orientation));
    result.setfield("windows", vw::octave::vector_to_octave(windows));
    return result;
  }
  #endif
};

struct MVPPixelResult : public MVPAlgorithmVar {
  vw::float32 variance;
  bool converged;
  int num_iterations;

  MVPPixelResult(MVPAlgorithmVar const& mav = MVPAlgorithmVar(), vw::float32 v = 0, bool c = true, int n = 0) :
    MVPAlgorithmVar(mav), variance(v), converged(c), num_iterations(n) {}

  #if MVP_ENABLE_OCTAVE_SUPPORT
  MVPPixelResult(::octave_value_list oct_val_list) :
    MVPAlgorithmVar(oct_val_list(0).scalar_map_value()),
    variance(oct_val_list(1).float_value()),
    converged(oct_val_list(2).bool_value()),
    num_iterations(int(oct_val_list(3).double_value())) {}
  #endif
};


struct MVPTileResult {
  vw::cartography::GeoReference georef;

  vw::ImageView<vw::PixelMask<vw::float32> > alt;
  vw::ImageView<vw::PixelMask<vw::float32> > variance;

  vw::ImageView<vw::PixelMask<vw::Vector3f> > orientation;
  vw::ImageView<vw::PixelMask<vw::Vector3f> > windows;

  MVPTileResult(vw::cartography::GeoReference g, int tile_size) : georef(g),
    alt(tile_size, tile_size), variance(tile_size, tile_size), 
    orientation(tile_size, tile_size), windows(tile_size, tile_size) {}

  inline void update(int col, int row, MVPPixelResult const& px_result) {
    alt(col, row) = px_result.alt;
    variance(col, row) = px_result.variance;

    orientation(col, row) = px_result.orientation;
    windows(col, row) = px_result.windows;

    if (!px_result.converged) {
      alt(col, row).invalidate();
      variance(col, row).invalidate();
      orientation(col, row).invalidate();
      windows(col, row).invalidate();
    }
  }
};

struct MVPSeedBBox {
  MVPAlgorithmVar seed;
  vw::BBox2i bbox;
  double alt_range;

  MVPSeedBBox(MVPAlgorithmVar s, vw::BBox2i b, double a) : seed(s), bbox(b), alt_range(a) {}
};

template <class ImplT>
struct MVPJobBase {

  static ImplT construct_from_job_request(MVPJobRequest const& job_request) {
    // TODO: This is common code
    int col = job_request.col();
    int row = job_request.row();
    int level = job_request.level();

    vw::platefile::PlateGeoReference plate_georef(job_request.plate_georef());

    vw::cartography::GeoReference georef(plate_georef.tile_georef(col, row, level));

    vw::BBox2 tile_bbox(plate_georef.tile_lonlat_bbox(col, row, level));
    vw::Vector2 alt_limits(job_request.user_settings().alt_min(), job_request.user_settings().alt_max());

    OrbitalImageCropCollection crops(tile_bbox, georef.datum(), alt_limits);
    crops.add_image_collection(job_request.orbital_images());

    return ImplT(georef, plate_georef.tile_size(), crops, job_request.user_settings());
  }

  inline ImplT& impl() {return static_cast<ImplT&>(*this);}
  inline ImplT const& impl() const {return static_cast<ImplT const&>(*this);}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, vw::cartography::GeoReference const& georef, MVPAlgorithmOptions const& options) const 
    {return impl().process_pixel(seed, georef, options);}

  inline MVPPixelResult process_pixel(MVPAlgorithmVar const& seed, double col, double row, MVPAlgorithmOptions const& options) const
    {return impl().process_pixel(seed, vw::cartography::crop(m_georef, col, row), options);}

  inline MVPPixelResult process_tangent_seed(vw::BBox2 bbox, double alt, double alt_range) const {
    VW_ASSERT(m_georef.datum().semi_major_axis() == m_georef.datum().semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 

    vw::Vector2 center_pt = (bbox.min() + bbox.max() - vw::Vector2(1, 1)) / 2;
    vw::Vector2 seed_lonlat = m_georef.pixel_to_lonlat(vw::Vector2(center_pt.x(), center_pt.y()));
    vw::Vector3f orientation = vw::cartography::lon_lat_radius_to_xyz(vw::Vector3(seed_lonlat[0], seed_lonlat[1], 1));
    vw::Vector3f windows = vw::Vector3f(bbox.width(), bbox.height(), m_settings.seed_window_smooth_size()) / 6;

    MVPAlgorithmVar seed(alt, orientation, windows);

    MVPAlgorithmOptions opts;
    opts.set_alt_range(alt_range);

    return process_pixel(seed, center_pt.x(), center_pt.y(), opts);
  }

  inline std::list<MVPSeedBBox> generate_seeds() const {

    int num_levels = 0;
    for (int i = m_tile_size; i >= m_settings.seed_window_size(); i /= 2) {
      num_levels++;
    }

    double top_level_alt_range = (1 << num_levels) * m_settings.alt_search_range();

    std::list<MVPSeedBBox> seed_list;
    for (int level = 0; level < num_levels; level++) {
      if (level == 0) {
        vw::BBox2 first_bbox(0, 0, m_tile_size, m_tile_size);

        double first_alt = (m_settings.alt_min() + m_settings.alt_max()) / 2;
        double first_alt_range = (m_settings.alt_max() - m_settings.alt_min()) / 2;

        MVPPixelResult first_result(process_tangent_seed(first_bbox, first_alt, first_alt_range));

        if (!first_result.converged) {
          break;        
        }

        // TODO: switch seed and bbox
        seed_list.push_back(MVPSeedBBox(first_result, first_bbox, top_level_alt_range / 2));
      } else {
        std::list<MVPSeedBBox> new_seed_list;

        BOOST_FOREACH(MVPSeedBBox const& sb, seed_list) {
          vw::Vector2 bbox_center = (sb.bbox.min() + sb.bbox.max()) / 2;

          std::list<vw::BBox2> subdivide_bboxes;
          subdivide_bboxes.push_back(vw::BBox2(sb.bbox.min(), bbox_center));
          subdivide_bboxes.push_back(vw::BBox2(bbox_center, sb.bbox.max()));
          subdivide_bboxes.push_back(vw::BBox2(vw::Vector2(sb.bbox.min().x(), bbox_center.y()), vw::Vector2(bbox_center.x(), sb.bbox.max().y())));
          subdivide_bboxes.push_back(vw::BBox2(vw::Vector2(bbox_center.x(), sb.bbox.min().y()), vw::Vector2(sb.bbox.max().x(), bbox_center.y())));

          BOOST_FOREACH(vw::BBox2 const& b, subdivide_bboxes) {
            MVPPixelResult subbox_result(process_tangent_seed(b, sb.seed.alt, sb.alt_range));
            if (subbox_result.converged) {
              new_seed_list.push_back(MVPSeedBBox(subbox_result, b, sb.alt_range / 2));
            }
          }
        }
        seed_list = new_seed_list;

        // TODO: warn if seed_list is empty here, because there should be at least one valid
        // bbox if the parent bbox converged...
      }
    }

    // Set seed windows to their final size
    BOOST_FOREACH(MVPSeedBBox& sb, seed_list) {
      sb.seed.windows = vw::Vector3f(m_settings.seed_window_size(), m_settings.seed_window_size(), m_settings.seed_window_smooth_size()) / 6;
    }

    return seed_list;
  }

  inline MVPTileResult process_tile(std::list<MVPSeedBBox> const& seed_list, 
                                    vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const {
    MVPTileResult tile_result(m_georef, m_tile_size);

    // Calcualte total # of pixels
    int num_px_to_process = 0; 
    BOOST_FOREACH(MVPSeedBBox const& sb, seed_list) {
      num_px_to_process += sb.bbox.width() * sb.bbox.height(); 
    }

    int curr_px_num = 0;
    BOOST_FOREACH(MVPSeedBBox const& sb, seed_list) {
      MVPAlgorithmOptions opts;
      opts.set_alt_range(sb.alt_range);
      //opts.set_fix_orientation(false);
      //opts.set_fix_windows(false);
      //opts.set_fast_reflectance(false);

      for (int col = sb.bbox.min().x(); col < sb.bbox.max().x(); col++) {
        for (int row = sb.bbox.min().y(); row < sb.bbox.max().y(); row++) {
          progress.report_fractional_progress(curr_px_num++, num_px_to_process);
          tile_result.update(col, row, process_pixel(sb.seed, col, row, opts));
          //tile_result.update(col, row, MVPPixelResult(sb.seed, 0, true));
        }
      }
    } 
    progress.report_finished();
    return tile_result;
  }

  inline MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const {
    return process_tile(generate_seeds(), progress);
  }

  protected:
    vw::cartography::GeoReference m_georef;
    int m_tile_size;
    OrbitalImageCropCollection m_crops;
    MVPUserSettings m_settings;
    // TODO: A seed object...

    // This is defined here to prevent the user from accidently
    // constructing an MVPJobBase
    MVPJobBase(vw::cartography::GeoReference const& georef, int tile_size, OrbitalImageCropCollection const& crops, MVPUserSettings const& settings) :
      m_georef(georef), m_tile_size(tile_size), m_crops(crops), m_settings(settings) {}
    MVPJobBase(MVPJobBase const& job) : m_georef(job.m_georef), m_tile_size(job.m_tile_size), m_crops(job.m_crops), m_settings(job.m_settings) {}

  private:
    // These are defined here to prevent them from being used
    MVPJobBase& operator=(MVPJobBase const&) {return *this;}
};

} // namespace mvp

#endif
