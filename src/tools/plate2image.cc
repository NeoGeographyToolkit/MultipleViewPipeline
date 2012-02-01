#include <iostream>

#include <vw/Plate/PlateView.h>

#include <boost/program_options.hpp>

using namespace std;

using namespace vw;
using namespace vw::platefile;

namespace po = boost::program_options;

struct Options {
  string plate_filename;
};

void handle_arguments(int argc, char *argv[], Options *opts) {

}

template <class PixelT>
void plate2image(boost::shared_ptr<PlateFile> platefile, Options const& opts) {


}

int main(int argc, char *argv[]) {
  Options opts;

  try {
    handle_arguments(argc, argv, &opts);

    boost::shared_ptr<PlateFile> platefile(new PlateFile(opts.plate_filename));

     PixelFormatEnum pixel_format = platefile->pixel_format();
     ChannelTypeEnum channel_type = platefile->channel_type();

     switch (pixel_format) {
       case VW_PIXEL_GRAYA:
         switch(channel_type) {
           case VW_CHANNEL_UINT8:
             plate2image<PixelGrayA<uint8> >(platefile, opts);
             break;
           case VW_CHANNEL_INT16:
             plate2image<PixelGrayA<int16> >(platefile, opts);
             break;
           case VW_CHANNEL_FLOAT32:
             plate2image<PixelGrayA<float32> >(platefile, opts);
             break;
           default:
             vw_throw(ArgumentErr() << "Platefile contains a channel type not supported by plate2image.");
         }
         break;
       case VW_PIXEL_RGB:
       case VW_PIXEL_RGBA:
       default:
         switch(channel_type) {
           case VW_CHANNEL_UINT8:
             plate2image<PixelRGBA<uint8> >(platefile, opts);
             break;
           default:
             vw_throw(ArgumentErr() << "Platefile contains a channel type not supported by plate2image.");
         }
         break;
     }
  } catch (ArgumentErr const& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  return 0;
}
