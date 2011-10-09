#include <vwoctave/Conversions.h>

DEFUN_DLD(loadcam_vw, args, ,
         "Load a camera using VW")
{
   octave_value_list retval;

   const int nargin = args.length();

   if (nargin != 1) {
     error("Must supply filename");
     return retval;
   }

   if (!args(0).is_string()) {
     error("Argument must be a filename");
     return retval;
   }


   std::string filename = args(0).string_value();

   Matrix oct_cam;
   try {
    vw::camera::PinholeModel vw_cam(filename);
    oct_cam = vw::octave::pinhole_to_octave(vw_cam);
   } catch (vw::Exception& e) {
    error(e.what());
    return retval;
   }

   return octave_value(oct_cam);
}
