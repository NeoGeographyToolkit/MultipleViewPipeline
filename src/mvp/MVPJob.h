/// \file MVPJob.h
///
/// MVP Job class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOB_H__
#define __MVP_MVPJOB_H__

#include <mvp/MVPJobImpl.h>

#include <vw/Plate/PlateFile.h>
#include <vw/Image/MaskViews.h>

#include <boost/filesystem.hpp>

namespace mvp {

class MVPJob {
  boost::shared_ptr<MVPJobImplBase> m_impl;
  MVPJobRequest m_job_request;
  int m_col, m_row, m_level;
  std::string m_result_platefile, m_internal_result_platefile;
  vw::platefile::PlateGeoReference m_plate_georef;

  public:
    MVPJob(MVPJobRequest const& job_request) :
      m_job_request(job_request),
      m_col(job_request.col()), m_row(job_request.row()), m_level(job_request.level()),
      m_result_platefile(job_request.result_platefile()),
      m_internal_result_platefile(job_request.internal_result_platefile()),
      m_plate_georef(job_request.plate_georef()) {
      
      vw::cartography::GeoReference georef = m_plate_georef.tile_georef(m_col, m_row, m_level);
      OrbitalImageCropCollection crops(m_plate_georef.tile_lonlat_bbox(m_col, m_row, m_level),
                                       m_plate_georef.datum(),
                                       job_request.user_settings().alt_min(),
                                       job_request.user_settings().alt_max());
      crops.add_image_collection(job_request.orbital_images());

      if (job_request.use_octave()) {
        #if MVP_ENABLE_OCTAVE_SUPPORT
          if (job_request.draw_footprints()) {
            m_impl.reset(new MVPJobImplFootprintOctave(georef, m_plate_georef.tile_size(), crops, job_request.user_settings()));
          } else {
            m_impl.reset(new MVPJobImplOctave(georef, m_plate_georef.tile_size(), crops, job_request.user_settings()));
          }
        #else
          vw::vw_throw(vw::NoImplErr() << "Cannot use octave algorithm, as the MVP was not compiled with it!");
        #endif
      } else if (job_request.draw_footprints()) {
        m_impl.reset(new MVPJobImplFootprint(georef, m_plate_georef.tile_size(), crops, job_request.user_settings()));
      } else {
        m_impl.reset(new MVPJobImpl(georef, m_plate_georef.tile_size(), crops, job_request.user_settings()));
      }
    }

    static MVPJobRequest load_job_file(std::string const& filename) {
      MVPJobRequest job_request;

      std::fstream input((filename + "/job").c_str(), std::ios::in | std::ios::binary);      
      if (!input) {
        vw_throw(vw::IOErr() << "Missing: /job");
      } else if (!job_request.ParseFromIstream(&input)) {
        vw_throw(vw::IOErr() << "Unable to process /job");
      }

      BOOST_FOREACH(mvp::OrbitalImageFileDescriptor& o, *job_request.mutable_orbital_images()) {
        o.set_image_path(filename + "/" + o.image_path());
        o.set_camera_path(filename + "/" + o.camera_path());
      }

      return job_request;
    }

    MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance(), bool write_tile = false) const {
      MVPTileResult result = m_impl->process_tile(progress);

      if (write_tile) {
        vw::ImageViewRef<vw::PixelGrayA<vw::float32> > rendered_tile = vw::mask_to_alpha(vw::pixel_cast<vw::PixelMask<vw::PixelGray<vw::float32> > >(result.alt));

        boost::scoped_ptr<vw::platefile::PlateFile> pf(new vw::platefile::PlateFile(m_result_platefile));

        pf->transaction_begin("Post Heights", 1);
        pf->write_request();
        pf->write_update(rendered_tile, m_col, m_row, m_level);
        pf->sync();
        pf->write_complete();
        pf->transaction_end(true);

        // TODO: Write the other tiles
      }

      return result;
    }

    std::string save_job_file(std::string const& out_dir = ".") {
      namespace fs = boost::filesystem;

      std::string job_filename;

      {
        std::stringstream stream;
        stream << out_dir << "/" << m_col << "_" << m_row << "_" << m_level << ".job";
        job_filename = stream.str();
      }

      // TODO: check IO errors
      fs::create_directory(job_filename);

      MVPJobRequest job_request_mod(m_job_request);
      for (unsigned curr_image = 0; curr_image < m_impl->crops().size(); curr_image++) {
        std::stringstream stream;
        stream << curr_image;
        std::string str_num(stream.str());

        std::string image_name("image" + str_num + ".tif");
        std::string camera_name("camera" + str_num + ".pinhole");

        job_request_mod.mutable_orbital_images(curr_image)->set_camera_path(camera_name);
        job_request_mod.mutable_orbital_images(curr_image)->set_image_path(image_name);

        vw::write_image(job_filename + "/" + image_name, m_impl->crops()[curr_image]);
        m_impl->crops()[curr_image].camera().write(job_filename + "/" + camera_name);
      }

      {
        std::fstream output((job_filename + "/job").c_str(), std::ios::out | std::ios::trunc | std::ios::binary);
        if (!job_request_mod.SerializeToOstream(&output)) {
          vw_throw(vw::IOErr() << "Failed to serialize jobfile");
        }
      }

      return job_filename;
    }
};

} // namespace mvp

#endif
