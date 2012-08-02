#include <mvp/Frontend/Session.h>

#include <vw/Plate/PlateGeoReference.h>

namespace mvp {
namespace frontend {

void Session::reset(SessionDesc const& session_desc) {
  m_session_desc = session_desc;

  // TODO: Create new OrbitalImageFootprintCollection

  vw::cartography::Datum datum(session_desc.output().datum());
  vw::platefile::PlateGeoReference plate_georef(datum, 
                                                session_desc.output().map_projection(),
                                                session_desc.output().tile_size(), 
                                                vw::cartography::GeoReference::PixelAsPoint);
  m_plate_georef_desc = plate_georef.build_desc();

  // Init render_bbox
  m_render_bbox = core::parse_bbox_string(session_desc.render().bbox());

  // Init cursor
  m_cursor = m_render_bbox.min();
}

pipeline::JobDesc Session::next() {
  using namespace pipeline;

  static int curr_id = 0;

  JobDesc::Input input;
  // TODO: use cursor position to get orbital images

  JobDesc::Render render;
  render.set_col(m_cursor.x());
  render.set_row(m_cursor.y());
  render.set_level(m_session_desc.render().level());
  render.set_use_octave(m_session_desc.render().use_octave());

  JobDesc::Output output;
  output.set_result(m_session_desc.output().result());
  output.set_internal_result(m_session_desc.output().internal_result());
  *output.mutable_plate_georef() = m_plate_georef_desc;

  JobDesc job_desc;
  job_desc.set_id(curr_id++);
  *job_desc.mutable_input() = input;
  *job_desc.mutable_render() = render;
  *job_desc.mutable_output() = output;
  *job_desc.mutable_algorithm() = m_session_desc.algorithm();

  if (!job_desc.IsInitialized()) {
    vw::vw_throw(vw::LogicErr() << "JobDesc missing fields");
  }

  // increment cursor
  m_cursor.x()++;
  if (!m_render_bbox.contains(m_cursor)) {
    m_cursor.x() = m_render_bbox.min().x();
    m_cursor.y()++;
  }

  return job_desc;
}

}} // namespace frontend, mvp
