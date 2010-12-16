#include <octave/oct.h>

#include <vw/Image.h>
#include <vw/FileIO.h>
 
DEFUN_DLD(imread_vw, args, ,
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

   Matrix oct_img(vw_img.rows(), vw_img.cols());

   typedef vw::ImageView<double>::pixel_accessor AccT;
   AccT racc = vw_img.origin();

   for(int row = 0; row < vw_img.rows(); row++) {
     AccT cacc = racc;
     for(int col = 0; col < vw_img.cols(); col++) {
       oct_img(row, col) = *cacc;
       cacc.next_col();
     }
     racc.next_row();
   }

   return octave_value(oct_img);
}
