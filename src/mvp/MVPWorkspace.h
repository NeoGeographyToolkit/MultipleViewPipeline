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

#ifndef __MVPWORKSPACE_H__
#define __MVPWORKSPACE_H__

namespace mvp {

class MVPWorkspace {
  vw::cartography::Datum m_datum;
  vw::Vector2 m_post_height_limits;
  MVPOperationDesc m_operation_desc;
  int m_tile_size;

  OrbitalImageCollection m_images;
  int m_equal_resolution_level, m_equal_density_level;
  
  public:
    MVPWorkspace(int vw::cartography::Datum const& datum, vw::Vector2 const& post_height_limits, 
                 MVPOperationDesc const& operation_desc, int tile_size = 256) :
      m_datum(datum), m_post_height_limits(post_height_limits), m_operation_desc(operation_desc),
      m_tile_size(tile_size), m_images(),
      m_equal_resolution_level(0), m_equal_density_level(0) {}

    /// Add an orbital image to the workspace
    void add_image(std::string const& camera_path, std::string const& image_path) {
      // m_images.push_back(OrbitalImage(camera_path, image_path));
    }

    /// Add a set of orbital images to the workspace based on a pattern
    void add_image_pattern(std::string const& camera_pattern, std::string const& image_pattern, vw::Vector2i const& range) {

    }

    /// Return the level at which the resolution of the orbital image is 
    /// approximately equal to the resolution of a tile in the platefile
    int equal_resolution_level() const {return m_equal_resolution_level;}

    /// Return the level at which the pixel density (measured in pixels 
    /// per degree) of the orbital image is approximately equal to the pixel 
    /// density of a tile in the platefile
    int equal_density_level() const {return m_equal_density_level;}

    /// Return a bounding box (in lonlat) that contains all of the orbital 
    /// imagery in the workspace
    vw::BBox2 lonlat_work_area() const {
      vw::BBox2 lonlat_bbox;
      return lonlat_bbox;
    }

    /// Return a bounding box (in tiles) that contains all of the orbital
    /// imagery in the workspace at a given level
    vw::BBox2i tile_work_area(int level) const {
      vw::BBox2i tile_bbox;
      return tile_bbox;
    } 

    /// Return a bounding box (in pixels) that contains all of the orbital
    /// imagery in the workspace at a given level
    vw::BBox2i pixel_work_area(int level) const {
      vw::BBox2i pixel_bbox;
      return pixel_bbox;
    }

    /// Return the georef for the entire workspace at a given level. This
    /// georef converts between the workspace's pixel coords and lonlat
    vw::cartography::GeoReference level_georef(int level) const {
      return vw::cartography::GeoReference();
    }

    /// Return the georef for a given tile in the workspace at a given level.
    /// This georef coverts between a tile's pixel coords and lonlat
    vw::cartography::GeoReference tile_georef(int col, int row, int level) const {
      return vw::cartography::GeoReference();
    }

    /// Return all the orbital images that overlap a given tile
    OrbitalImageCollection images_at_tile(int col, int row, int level) const {
      return OrbitalImageCollection();
    }

    /// Return an MVPJobRequest for a given tile at a given level.
    MVPJobRequest assemble_job(int col, int row, int level) const {
      // MVPJobRequest will be a protobuf class
      return MVPJobRequest();
    }
};

} // namespace mvp
#endif
