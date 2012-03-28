#include <mvp/MVPJob.h>

#include <boost/filesystem.hpp>

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

  if (nargout != 4) {
    error("Must supply four output args");
    return retval;
  }

  std::string filename = args(0).string_value();
  if (!fs::is_directory(filename)) {
    error("Not a valid jobfile");
    return retval;
  }

  mvp::MVPJobRequest job_request;

  try {
    job_request = mvp::MVPJob::load_job_file(filename);
  } catch (vw::Exception& e) {
    error(e.what());
  }

  vw::cartography::GeoReference georef(job_request.georef());

  // TODO: Catch exception when images are not found
  mvp::OrbitalImageCropCollection crops(georef, job_request.tile_size(), job_request.user_settings().alt_min(), job_request.user_settings().alt_max());
  crops.add_image_collection(job_request.orbital_images());

  retval.append(vw::octave::georef_to_octave(georef));
  retval.append(crops.to_octave());
  retval.append(vw::octave::protobuf_to_octave(&job_request.user_settings()));
  retval.append(job_request.tile_size());

  return retval;
}
