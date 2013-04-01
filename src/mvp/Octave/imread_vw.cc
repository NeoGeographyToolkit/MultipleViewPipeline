#include <octave/oct.h> 

#include <mvp/Octave/Conversions.h>

#include <vw/FileIO.h>

DEFUN_DLD(imread_vw, args, nargout, "Load mvp functions/objects") {
  using namespace vw;
  using namespace mvp;
  ImageView<PixelGrayA<double> > image;

  if (args.length() != 1) {
    error("Usage: imread_vw(filename)");
    return octave_value();
  }

  read_image(image, args(0).string_value());

  return octave::to_octave(image);
}
