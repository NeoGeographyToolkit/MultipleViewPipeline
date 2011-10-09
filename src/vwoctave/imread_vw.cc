#include <vwoctave/Conversions.h>

DEFUN_DLD(imread_vw, args, nargout,
         "Load an image using VW")
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

   vw::ImageView<double> vw_img;
   try {
    read_image(vw_img, filename);
   } catch (vw::Exception& e) {
    error(e.what());
    return retval;
   }

   Matrix oct_img(vw::octave::imageview_to_octave(vw_img));

   if (nargout == 1) {
     return octave_value(oct_img);
   }

   vw::cartography::GeoReference vw_geo;
   try {
     read_georeference(vw_geo, filename);
   } catch (vw::Exception& e) {
     error(e.what());
     return retval;
   }

   vw::Matrix3x3 vw_geo_trans = vw_geo.transform();
   Matrix oct_geo(3, 3);

   for (int r = 0; r < 3; r++) {
     for (int c = 0; c < 3; c++) {
       oct_geo(r, c) = vw_geo_trans(r, c);
     }
   }

   Matrix rebase = identity_matrix(3, 3);
   rebase(0, 2) = rebase(1, 2) = 1;
   oct_geo = oct_geo * rebase.inverse();

   Matrix deg2rad = identity_matrix(3, 3);
   deg2rad(0, 0) = deg2rad(1, 1) = M_PI / 180.0;
   oct_geo = deg2rad * oct_geo;

   retval.append(oct_img);
   retval.append(oct_geo);
   return retval;
}
