#include <octave/oct.h>
 
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


   return octave_value(5);
}
