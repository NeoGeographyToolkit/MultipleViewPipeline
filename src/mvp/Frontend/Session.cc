#include <mvp/Frontend/Session.h>
#include <mvp/Core/Settings.h> //parse_bbox_string

#include <vw/Plate/PlateGeoReference.h>

namespace mvp {
namespace frontend {

void Session::reset(SessionDesc const& session_desc) {
  m_session_desc = session_desc;

  vw::cartography::Datum datum(session_desc.output().datum());
  //m_footprints.reset(new geometry::FootprintCollection(datum, session_desc.algorithm().alt_min(),
  //                                                            session_desc.algorithm().alt_max()));
  //m_footprints->push_back_pattern(session_desc.input().image_pattern(), session_desc.input().camera_pattern());

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

pipeline::JobDesc Session::next_job() {
  using namespace pipeline;

  static int curr_id = 0;

  // Init objects
  int col = m_cursor.x();
  int row = m_cursor.y();
  int level = m_session_desc.render().level();

  vw::platefile::PlateGeoReference plate_georef(m_plate_georef_desc);
  vw::BBox2 lonlat_bbox = plate_georef.tile_lonlat_bbox(col, row, level);
//  std::vector<image::OrbitalImageDesc> orbital_image_descs = m_footprints->images_in_region(lonlat_bbox);

  // Fill out JobDesc
  JobDesc::Input input;
//  std::copy(orbital_image_descs.begin(), orbital_image_descs.end(), RepeatedFieldBackInserter(input.mutable_orbital_images()));

  JobDesc::Render render;
  render.set_col(col);
  render.set_row(row);
  render.set_level(level);

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
