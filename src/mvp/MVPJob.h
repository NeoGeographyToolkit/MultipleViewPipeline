/// \file MVPJob.h
///
/// MVP Job class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOB_H__
#define __MVP_MVPJOB_H__

#include <mvp/MVPJobImpl.h>

// TODO: Remove PlateGeoReference from MVPJob and MVPJobRequest.
// simply pass a GeoReference and tile size
#include <vw/Plate/PlateGeoReference.h>

namespace mvp {

class MVPJob {
  boost::shared_ptr<MVPJobImplBase> m_impl;
  MVPJobRequest m_job_request;
  int m_col, m_row, m_level;
  std::string m_result_platefile, m_internal_result_platefile;
  vw::platefile::PlateGeoReference m_plate_georef;

  public:
    MVPJob(MVPJobRequest const& job_request); 

    static MVPJobRequest load_job_file(std::string const& filename); 

    MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance(), bool write_tile = false) const;

    std::string save_job_file(std::string const& out_dir = "."); 
};

} // namespace mvp

#endif
