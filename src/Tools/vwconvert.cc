#include <iostream>
#include <vw/Image.h>
#include <vw/FileIO.h>

using namespace vw;

int main(int argc, char *argv[]) {
  try {
    VW_ASSERT(argc==3, ArgumentErr() << "Invalid command-line args.");
    ImageView<float32> image;
    read_image(image, argv[1]);
    write_image(argv[2], channel_cast_rescale<uint8>(normalize(image)));
  } catch (Exception& e) {
    std::cerr << "Error: " << e.what() << std::endl;
    std::cerr << "Usage: vwconvert <src> <dest>" << std::endl;
    return 1;
  }
  return 0;
}
    
