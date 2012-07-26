/// \file Session.h
///
/// Session class
///

#ifndef __MVP_PIPELINE_SESSION_H__
#define __MVP_PIPELINE_SESSION_H__

#include <vw/Math/BBox.h>

#include <mvp/Pipeline/SessionDesc.pb.h>
#include <mvp/Pipeline/JobDesc.pb.h>

namespace mvp {
namespace pipeline {

class Session {
  SessionDesc m_session_desc;
  vw::Vector2i m_cursor;
  vw::BBox2i m_render_bbox;

  public:
    Session(SessionDesc const& session_desc) {
      reset(session_desc);
    }

    void reset(SessionDesc const& session_desc) {
      m_session_desc = session_desc;
      // Create new OrbitalImageFootprintCollection

      // Init render_bbox

      // Init cursor
      m_cursor = m_render_bbox.min();
    }

    bool has_next() {
      return m_render_bbox.contains(m_cursor);
    }

    JobDesc next() {
      JobDesc job_desc;
      // use cursor position to get tile and build JobDesc

      // increment cursor
      m_cursor.x()++;
      if (!m_render_bbox.contains(m_cursor)) {
        m_cursor.x() = m_render_bbox.min().x();
        m_cursor.y()++;
      }

      return JobDesc();
    }

/*
    void update_status(StatusUpdate const& update) {

    }
*/
};

}} // namespace pipeline, mvp

#endif
