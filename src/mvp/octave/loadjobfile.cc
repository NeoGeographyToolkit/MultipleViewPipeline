#include <mvp/MVPJob.h>

DEFUN_DLD(loadjobfile, args, nargout, "Load an MVP Job File")
{
  namespace fs = boost::filesystem;

  octave_value_list retval;

  const int nargin = args.length();

  if (nargin != 1) {
    error("Must supply job filename");
    return retval;
  }

  if (!args(0).is_string()) {
    error("Argument must be a filename");
    return retval;
  }

  if (nargout != 3) {
    error("Must supply three output args");
    return retval;
  }

  std::string filename = args(0).string_value();
  if (!fs::is_directory(filename)) {
    error("Not a valid jobfile");
    return retval;
  }

  mvp::MVPJobRequest job_request;
  {
    std::fstream input((filename + "/job").c_str(), std::ios::in | std::ios::binary);      
    if (!input) {
      error("Missing: /job");
      return retval;
    } else if (!job_request.ParseFromIstream(&input)) {
      error("Unable to process /job");
      return retval;
    }
  }

  BOOST_FOREACH(mvp::OrbitalImageFileDescriptor& o, *job_request.mutable_orbital_images()) {
    o.set_image_path(filename + "/" + o.image_path());
    o.set_camera_path(filename + "/" + o.camera_path());
  }

  // TODO: this is common code 
  int col = job_request.col();
  int row = job_request.row();
  int level = job_request.level();

  vw::platefile::PlateGeoReference plate_georef(job_request.plate_georef());

  vw::cartography::GeoReference georef(plate_georef.tile_georef(col, row, level));

  vw::BBox2 tile_bbox(plate_georef.tile_lonlat_bbox(col, row, level));
  vw::Vector2 post_height_limits(job_request.algorithm_settings().post_height_limit_min(), job_request.algorithm_settings().post_height_limit_max());

  // TODO: Catch exception when images are not found
  mvp::OrbitalImageCropCollection crops(tile_bbox, georef.datum(), post_height_limits);
  crops.add_image_collection(job_request.orbital_images());

  retval.append(vw::octave::georef_to_octave(georef));
  retval.append(crops.to_octave());
  retval.append(vw::octave::protobuf_to_octave(&job_request.algorithm_settings()));

  return retval;
}
