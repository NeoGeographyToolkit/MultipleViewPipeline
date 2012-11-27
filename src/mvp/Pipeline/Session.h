/// \file Session.h
///
/// Session class
///

#ifndef __MVP_PIPELINE_SESSION_H__
#define __MVP_PIPELINE_SESSION_H__

#include <mvp/Pipeline/JobDesc.pb.h>
#include <mvp/Pipeline/SessionDesc.pb.h>

#include <vw/Math/Vector.h>
#include <vw/Math/BBox.h>

// Rather than including an ENTIRE PlateGeoReference, we
// just hold onto its desc.
#include <vw/Plate/PlateGeoReferenceDesc.pb.h>

namespace mvp {
namespace pipeline {

class Session {
  SessionDesc m_session_desc;
  vw::platefile::PlateGeoReferenceDesc m_plate_georef_desc;
//  boost::shared_ptr<geometry::FootprintCollection> m_footprints;
  vw::Vector2i m_cursor;
  vw::BBox2i m_render_bbox;

  public:
    Session() {}

    Session(SessionDesc const& session_desc) { reset(session_desc); }

    void reset(SessionDesc const& session_desc);

    bool has_next_job() { return m_render_bbox.contains(m_cursor); }

    pipeline::JobDesc next_job();

    int size() { return m_render_bbox.width() * m_render_bbox.height(); }
};

}} // namespace pipeline, mvp

#endif
