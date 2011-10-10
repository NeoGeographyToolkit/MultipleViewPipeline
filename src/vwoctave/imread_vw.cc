#include <vwoctave/Conversions.h>

DEFUN_DLD(imread_vw, args, nargout, "Load an image using VW")
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

   Matrix oct_geo;
   vw::cartography::GeoReference vw_geo;
   try {
     read_georeference(vw_geo, filename);
     oct_geo = vw::octave::georef_to_octave(vw_geo);
   } catch (vw::Exception& e) {
     error(e.what());
     return retval;
   }

   retval.append(oct_img);
   retval.append(oct_geo);
   return retval;
}
