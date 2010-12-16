#include <octave/oct.h>

#include <vw/Camera.h>
 
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

   vw::Matrix<double,3,4> vw_cam_mat;
   try {
    vw::camera::PinholeModel vw_cam(filename);
    vw_cam_mat = vw_cam.camera_matrix();
   } catch (vw::Exception& e) {
    error(e.what());
    return retval;
   }

   Matrix oct_cam_mat(3, 4);

   for (int r = 0; r < 3; r++) {
     for (int c = 0; c < 4; c++) {
       oct_cam_mat(r, c) = vw_cam_mat(r, c);
     }
   }

   return octave_value(oct_cam_mat);
}
