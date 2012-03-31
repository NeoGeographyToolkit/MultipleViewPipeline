#include <vw/Octave/Conversions.h>
#include <vw/Image/ImageViewRef.h>
#include <vw/FileIO/DiskImageView.h>

static vw::ImageViewRef<vw::PixelMask<vw::PixelGray<vw::float32> > > rsrc_helper(boost::shared_ptr<vw::DiskImageResource> rsrc) {
  switch(rsrc->format().pixel_format) {
    case vw::VW_PIXEL_GRAYA:
      return vw::DiskImageView<vw::PixelMask<vw::PixelGray<vw::float32> > >(rsrc);
      break;
    case vw::VW_PIXEL_GRAY:
      return vw::pixel_cast<vw::PixelMask<vw::PixelGray<vw::float32> > >(vw::DiskImageView<vw::PixelGray<vw::float32> >(rsrc));
      break;
    default:
      vw::vw_throw(vw::ArgumentErr() << "Unsupported orbital image pixel format: " << vw::pixel_format_name(rsrc->format().pixel_format));
  }
}

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

   boost::shared_ptr<vw::DiskImageResource> rsrc;
   try {
     rsrc.reset(vw::DiskImageResource::open(filename));
   } catch (vw::Exception& e) {
     error(e.what());
     return retval;
   }

   Matrix oct_img(vw::octave::imageview_to_octave(rsrc_helper(rsrc)));

   if (nargout == 1) {
     return octave_value(oct_img);
   }

   octave_scalar_map oct_geo;
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
