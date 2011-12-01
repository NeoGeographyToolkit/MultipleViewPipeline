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

#include <mvp/MVPAlgorithmSettings.pb.h>
#include <mvp/MVPJobRequest.pb.h>
#include <mvp/OrbitalImageFootprint.h>

#include <vw/Cartography/GeoReference.h>
#include <vw/Plate/PlateGeoReference.h>

#include <boost/program_options.hpp>

namespace mvp {

class MVPWorkspace {
  std::string m_result_platefile, m_internal_result_platefile;
  vw::platefile::PlateGeoReference m_plate_georef;
  MVPAlgorithmSettings m_algorithm_settings;
  OrbitalImageFootprintCollection m_footprints;
 
  public:
    MVPWorkspace(std::string const& result_platefile, std::string const& internal_result_platefile,
                 vw::platefile::PlateGeoReference const& plate_georef, 
                 MVPAlgorithmSettings const& algorithm_settings) :
      m_result_platefile(result_platefile), m_internal_result_platefile(internal_result_platefile),
      m_plate_georef(plate_georef), m_algorithm_settings(algorithm_settings),
      m_footprints(plate_georef.datum(), algorithm_settings.alt_min(), algorithm_settings.alt_max()) {}

    static boost::program_options::options_description program_options() {
      namespace po = boost::program_options;

      po::options_description options("Pipeline Configuration");
      options.add_options()
        ("orbital-image-pattern", po::value<std::string>()->required(), "Path to orbital images")
        ("camera-pattern", po::value<std::string>()->required(), "Path to orbital image camera models")
        ("pattern-index-start", po::value<int>()->required(), "Starting index to substitute into image/camera pattern")
        ("pattern-index-end", po::value<int>()->required(), "Ending index to substitute into image/camera pattern")
        ("datum", po::value<std::string>()->required(), "Datum name")
        ("map-projection", po::value<std::string>()->default_value("equi"), "Plate map projection type")
        ("tile-size", po::value<int>()->default_value(256), "Plate tile size")
        ("result-platefile", po::value<std::string>()->required(), "Result plate filename")
        ("internal-result-platefile", po::value<std::string>(), "Internal result plate filename")
        ("alt-min", po::value<double>()->required(), "Min DEM altitude to search")
        ("alt-max", po::value<double>()->required(), "Man DEM altitude to search")
        ("use-octave", po::value<bool>()->default_value(false), "Use octave in processing")
        ("test-algorithm", po::value<bool>()->default_value(false), "Run the test algorithm that draws footprints instead of creating a DEM")
        ;

      return options;
    }

    static MVPWorkspace construct_from_program_options(boost::program_options::variables_map const& vm) {
      vw::platefile::PlateGeoReference plate_georef(vw::cartography::Datum(vm["datum"].as<std::string>()), 
                                                    vm["map-projection"].as<std::string>(), 
                                                    vm["tile-size"].as<int>(), 
                                                    vw::cartography::GeoReference::PixelAsPoint);

      MVPAlgorithmSettings settings;
      settings.set_alt_min(vm["alt-min"].as<double>());
      settings.set_alt_max(vm["alt-max"].as<double>());
      settings.set_use_octave(vm["use-octave"].as<bool>());
      settings.set_test_algorithm(vm["test-algorithm"].as<bool>());
      // TODO: Add 'octave_function'

      MVPWorkspace work(vm["result-platefile"].as<std::string>(), vm["internal-result-platefile"].as<std::string>(), plate_georef, settings);
      work.add_image_pattern(vm["orbital-image-pattern"].as<std::string>(), 
                             vm["camera-pattern"].as<std::string>(),
                             vm["pattern-index-start"].as<int>(),
                             vm["pattern-index-end"].as<int>());
      return work;
    }

    /// Add an orbital image to the workspace
    void add_image(std::string const& image_path, std::string const& camera_path) {
      m_footprints.add_image(image_path, camera_path);
    }

    /// Add a set of orbital images to the workspace based on a pattern
    void add_image_pattern(std::string const& image_pattern, std::string const& camera_pattern, int start, int end) {
      m_footprints.add_image_pattern(image_pattern, camera_pattern, start, end);
    }

    /// Return the name of the result platefile for this workspace
    std::string result_platefile() const {return m_result_platefile;}

    /// Return the name of the internal result platefile for this workspace
    std::string internal_result_platefile() const {return m_internal_result_platefile;}

    /// Return the number of images in this workspace
    int num_images() const {return m_footprints.size();}

    /// Return the PlateGeoReference for the workspace
    vw::platefile::PlateGeoReference plate_georef() const {return m_plate_georef;}

    /// Return the level at which the resolution of the orbital image is 
    /// approximately equal to the resolution of a tile in the platefile
    int equal_resolution_level() const {return m_footprints.equal_resolution_level();}

    /// Return the level at which the pixel density (measured in pixels 
    /// per degree) of the orbital image is approximately equal to the pixel 
    /// density of a tile in the platefile
    int equal_density_level() const {return m_footprints.equal_density_level(m_plate_georef.tile_size());}

    /// Return a bounding box (in lonlat) that contains all of the orbital 
    /// imagery in the workspace
    vw::BBox2 lonlat_work_area() const {return m_footprints.lonlat_bbox();}

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
    std::vector<OrbitalImageFileDescriptor> images_at_tile(int col, int row, int level) const {
      return m_footprints.images_in_region(m_plate_georef.tile_lonlat_bbox(col, row, level));
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
      *request.mutable_algorithm_settings() = m_algorithm_settings;

      std::vector<OrbitalImageFileDescriptor> image_matches(images_at_tile(col, row, level));
      std::copy(image_matches.begin(), image_matches.end(), RepeatedFieldBackInserter(request.mutable_orbital_images()));

      return request;
    }
};

} // namespace mvp
#endif
