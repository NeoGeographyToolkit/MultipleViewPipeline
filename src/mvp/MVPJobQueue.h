/// \file MVPJobQueue.h
///
/// MVP Job Queue
///
/// TODO: Write something here
///

#include <mvp/MVPWorkspace.h>
#include <mvp/MVPMessages.pb.h>

#ifndef __MVP_MVPJOBQUEUE_H__
#define __MVP_MVPJOBQUEUE_H__

namespace mvp {

class MVPJobQueue {
  boost::shared_ptr<MVPWorkspace> m_work;
  vw::BBox2i m_render_bbox;
  int m_render_level;

  public:
    MVPJobQueue() : m_render_level(0) {}

    void reset(std::string const& mvp_conf) {
      std::cout << "Launching: " << mvp_conf << std::endl;
    }

    bool has_next() {
      return m_render_level < 3; // For now simulating sending out 3 tiles
    }

    MVPJobRequest next() {
      m_render_level++;
      return MVPJobRequest();
    }

    void update_status(MVPStatusUpdate const& update) {
      std::cout << "Received status update: " << update.status() << std::endl;
    }

    MVPStatusReport status() const {
      return MVPStatusReport();
    }
};

} // namespace mvp

#endif
