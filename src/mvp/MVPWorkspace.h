/// \file MVPWorkspace.h
///
/// MVP Workspace class
///
/// This class represents the work area of a multiple view mosaic job. After
/// a workspace is constructed, orbital images are added to it. Then, the
/// workspace can be queried at a specific tile and level using 
/// images_at_tile(), and all of the orbital images that possibly overlap
/// that tile are returned.
///
/// There are three ways of representing locations and BBoxes in the workspace.
/// The first is using lonlat coords, which are independent of level. In this
/// system the upper left hand corner of the workspace is (-180, 180) and the
/// lower right hand corner is (180, -180). Since BBoxes in VW require that
/// both arguments of the lower limit be smaller than the upper limit (because
/// VW assumes a coordinate system where right and down are positive, the BBox
/// for the entire workspace will actually be (-180, -180) to (180, 180).
///
/// The next representation is using pixel coords, which are dependant on
/// level. The size of an entire level in pixels will be (tile_size * 2^level)
/// on one side. Thus, the BBox of an entire level in this system will be 
/// (0, 0) to (tile_size * 2^level, tile_size * 2^level)
///
/// The last representation is using tile coords, which are also dependant on
/// level. The size of an entire level in tiles will be (2^level). Thus, the
/// BBox of an entire level in this system will be (0, 0) to (2^level, 2^level)
///

#ifndef __MVP_MVPWORKSPACE_H__
#define __MVP_MVPWORKSPACE_H__

#include <mvp/MVPJobRequest.pb.h>
#include <mvp/MVPWorkspaceRequest.pb.h>
#include <mvp/MVPUserSettings.pb.h>
#include <mvp/OrbitalImageFootprint.h>

#include <vw/Cartography/GeoReference.h>
#include <vw/Plate/PlateGeoReference.h>

namespace mvp {

//TODO: rework to allow for a boost::for_each to traverse every job_request in
//the render_bbox  
class MVPWorkspace {
  std::string m_result_platefile, m_internal_result_platefile;
  vw::platefile::PlateGeoReference m_plate_georef;
  MVPUserSettings m_user_settings;
  OrbitalImageFootprintCollection m_footprints;
  vw::BBox2i m_render_bbox;
  int m_render_level;
  bool m_use_octave;
  bool m_draw_footprints;
 
  public:
    MVPWorkspace(MVPWorkspaceRequest const& work_request) :
      m_result_platefile(work_request.result_platefile()), 
      m_internal_result_platefile(work_request.internal_result_platefile()),
      m_plate_georef(work_request.plate_georef()),
      m_user_settings(work_request.user_settings()),
      m_footprints(work_request.plate_georef().datum(), 
                   work_request.user_settings().alt_min(), 
                   work_request.user_settings().alt_max()),
      m_use_octave(work_request.use_octave()),
      m_draw_footprints(work_request.draw_footprints()) {
      
      m_footprints.add_image_collection(work_request.orbital_images());

      if (work_request.has_render_level()) {
        m_render_level = work_request.render_level();
      } else {
        m_render_level = m_footprints.equal_density_level(m_plate_georef.tile_size());
      }

      if (work_request.render_bbox().size() == 4) {
        m_render_bbox.min() = vw::Vector2i(work_request.render_bbox(0), work_request.render_bbox(1));
        m_render_bbox.max() = vw::Vector2i(work_request.render_bbox(2), work_request.render_bbox(3));
      } else {
        vw::BBox2 lonlat_bbox = m_footprints.lonlat_bbox();
        vw::BBox2 pixel_bbox = m_plate_georef.level_georef(m_render_level).lonlat_to_pixel_bbox(lonlat_bbox);
        m_render_bbox = vw::grow_bbox_to_int(pixel_bbox / m_plate_georef.tile_size());
      }
    }

    /// Return the user settings associated with this workspace
    MVPUserSettings const& user_settings() const {return m_user_settings;}

    /// Return the name of the result platefile for this workspace
    std::string const& result_platefile() const {return m_result_platefile;}

    /// Return the name of the internal result platefile for this workspace
    std::string const& internal_result_platefile() const {return m_internal_result_platefile;}

    /// Return the PlateGeoReference for the workspace
    vw::platefile::PlateGeoReference const& plate_georef() const {return m_plate_georef;}

    /// Return the OrbitalImageFootprints for the workspace
    OrbitalImageFootprintCollection const& footprints() const {return m_footprints;}

    /// Return the (tile) render bbox for the workspace
    vw::BBox2i const& render_bbox() const {return m_render_bbox;}

    /// Return the lonlat render bbox for the workspace
    vw::BBox2 render_lonlat_bbox() const {return m_plate_georef.tile_lonlat_bbox(m_render_bbox, m_render_level);}

    /// Return the render level for the worspace
    int render_level() const {return m_render_level;}

    bool use_octave() const {return m_use_octave;}

    bool draw_footprints() const {return m_draw_footprints;}

    /// Return all the orbital images that overlap a given tile
    std::vector<OrbitalImageFileDescriptor> images_at_tile(int col, int row, int level) const {
      return m_footprints.images_in_region(m_plate_georef.tile_lonlat_bbox(col, row, level));
    }

    /// Return all orbital images overlapping a tile bbox region
    std::vector<OrbitalImageFileDescriptor> images_at_tile_bbox(vw::BBox2i tile_bbox, int level) const {
      return m_footprints.images_in_region(m_plate_georef.tile_lonlat_bbox(tile_bbox, level));
    }

    /// Return an MVPJobRequest for a given tile at a given level.
    MVPJobRequest assemble_job(int col, int row, int level) const {
      using google::protobuf::RepeatedFieldBackInserter;

      MVPJobRequest request;

      request.set_col(col);
      request.set_row(row);
      request.set_level(level);
      
      request.set_result_platefile(m_result_platefile);
      request.set_internal_result_platefile(m_internal_result_platefile);
      *request.mutable_plate_georef() = m_plate_georef.build_desc();
      *request.mutable_user_settings() = m_user_settings;
      request.set_use_octave(m_use_octave);
      request.set_draw_footprints(m_draw_footprints);

      std::vector<OrbitalImageFileDescriptor> image_matches(images_at_tile(col, row, level));
      std::copy(image_matches.begin(), image_matches.end(), RepeatedFieldBackInserter(request.mutable_orbital_images()));

      return request;
    }
};

} // namespace mvp
#endif
