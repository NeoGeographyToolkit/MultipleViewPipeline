#include <octave/oct.h> 

#include <mvp/Pipeline/Job.h>

#include <mvp/Octave/Main.h>
#include <mvp/Octave/Conversions.h>

DEFUN_DLD(loadjobfile, args, nargout, "Load mvp functions/objects") {
  static bool mvp_registered = false;

  if (!mvp_registered) {
    mvp::octave::register_octave_mvp();
    mvp_registered = true;
  }

  if (args.length() != 1) {
    error("Usaged job = loadjobfile([path])");
    return octave_value();
  }

  mvp::pipeline::Job job(args(0).string_value());

  octave_map result;

  result.setfield("orbital_images", mvp::octave::to_octave(job.orbital_images()));
  result.setfield("algorithm_settings", mvp::octave::to_octave(job.algorithm_settings()));
  result.setfield("georef", mvp::octave::to_octave(job.georef()));
  result.setfield("tile_size", mvp::octave::to_octave(job.tile_size()));

  return octave_value(result);
}
