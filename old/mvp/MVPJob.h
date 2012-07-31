/// \file MVPJob.h
///
/// MVP Job class
///
/// TODO: Write something here
///

#ifndef __MVP_MVPJOB_H__
#define __MVP_MVPJOB_H__

#include <mvp/MVPTypes.h>
#include <mvp/MVPJobRequest.pb.h>
#include <mvp/OrbitalImageCrop.h>

namespace mvp {

class MVPJob {
  MVPJobRequest m_job_request;
  OrbitalImageCropCollection m_crops;

  public:
    MVPJob(MVPJobRequest const& job_request);

    MVPTileResult process_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const;

    void write_tile(MVPTileResult const& result) const;

    MVPTileResult process_and_write_tile(vw::ProgressCallback const& progress = vw::ProgressCallback::dummy_instance()) const;

    static MVPJobRequest load_job_file(std::string const& filename);

    std::string save_job_file(std::string const& out_dir = ".") const; 
};

} // namespace mvp

#endif
