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

#include "MVPOperationDesc.pb.h"
#include "MVPJobRequest.pb.h"
#include "OrbitalImage.h"

#include <vw/Cartography/GeoReference.h>
#include <vw/Plate/PlateGeoReference.h>
#include <vw/Image/Transform.h> // grow_bbox_to_int

#include <boost/format.hpp>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>

namespace mvp {

class MVPWorkspace {
  vw::platefile::PlateGeoReference m_plate_georef;
  vw::Vector2 m_post_height_limits;
  MVPOperationDesc m_operation_desc;
  OrbitalImageCollection m_images;

  int m_equal_resolution_level, m_equal_density_level;
  vw::BBox2 m_lonlat_work_area;
  
  public:
    MVPWorkspace(vw::platefile::PlateGeoReference const& plate_georef, 
                 MVPOperationDesc const& operation_desc, vw::Vector2 const& post_height_limits) :
      m_plate_georef(plate_georef), m_operation_desc(operation_desc),
      m_post_height_limits(post_height_limits), m_images(),
      m_equal_resolution_level(std::numeric_limits<int>::max()), m_equal_density_level(0), m_lonlat_work_area() {}

    /// Add an orbital image to the workspace
    void add_image(std::string const& camera_path, std::string const& image_path) {
      // TODO: Someday take into account Datums that are ellipsoid
      VW_ASSERT(m_plate_georef.datum().semi_major_axis() == m_plate_georef.datum().semi_minor_axis(),
        vw::ArgumentErr() << "Datum must be spheroid");

      vw::Vector2 radius_range = vw::Vector2(m_plate_georef.datum().radius(0, 0), m_plate_georef.datum().radius(0, 0)) + m_post_height_limits;

      OrbitalImage image(camera_path, image_path, radius_range);
      m_images.push_back(image);
      m_equal_resolution_level = std::min(m_equal_resolution_level, image.equal_resolution_level());
      m_equal_density_level = std::max(m_equal_density_level, image.equal_density_level());
      m_lonlat_work_area.grow(image.footprint_bbox());
    }

    /// Add a set of orbital images to the workspace based on a pattern
    void add_image_pattern(std::string const& camera_pattern, std::string const& image_pattern, vw::Vector2i const& range) {
      namespace fs = boost::filesystem;

      for (int i = range[0]; i <= range[1]; i++) {
        std::string camera_file = (boost::format(camera_pattern) % i).str();
        std::string image_file = (boost::format(image_pattern) % i).str();
        if (fs::exists(camera_file) && fs::exists(image_file)) {
          add_image(camera_file, image_file);
        } else {
          vw::vw_out(vw::DebugMessage, "mvp") << "Couldn't find " << camera_file << " or " << image_file;
        }
      }
    }

    int num_images() const {return m_images.size();}

    /// Return the PlateGeoReference for the workspace
    vw::platefile::PlateGeoReference plate_georef() const {return m_plate_georef;}

    /// Return the level at which the resolution of the orbital image is 
    /// approximately equal to the resolution of a tile in the platefile
    int equal_resolution_level() const {return m_equal_resolution_level;}

    /// Return the level at which the pixel density (measured in pixels 
    /// per degree) of the orbital image is approximately equal to the pixel 
    /// density of a tile in the platefile
    int equal_density_level() const {return m_equal_density_level;}

    /// Return a bounding box (in lonlat) that contains all of the orbital 
    /// imagery in the workspace
    vw::BBox2 lonlat_work_area() const {return m_lonlat_work_area;}

    /// Return a bounding box (in pixels) that contains all of the orbital
    /// imagery in the workspace at a given level
    vw::BBox2i pixel_work_area(int level) const {
      return m_plate_georef.level_georef(level).lonlat_to_pixel_bbox(lonlat_work_area());
    }

    /// Return a bounding box (in tiles) that contains all of the orbital
    /// imagery in the workspace at a given level
    vw::BBox2i tile_work_area(int level) const {
      vw::BBox2 bbox(pixel_work_area(level));
      bbox /= m_plate_georef.tile_size();
      return vw::grow_bbox_to_int(bbox);
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
