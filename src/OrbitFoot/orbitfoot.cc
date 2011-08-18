#include <iostream>

#include <vw/Image.h>
#include <vw/Cartography.h>
#include <vw/FileIO.h>
#include <vw/Camera.h>

#include "orbitfoot.h"

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

int main()
{
  using std::cout;
  using std::endl;
  using std::string;
  using namespace vw;

  string pinhole_file_fmt = "/byss/khusmann/data/AS15_cameras/AS15-M-%04d.lev1.pinhole";
  double moon_rad = 1737400.0;
  int tile_size = 256;

  std::vector<OrbitFootprint> orbits;
  BBox2 renderbox;

  for (int i = 73; i < 2492; i++) {
//  for (int i = 73; i < 74; i++) {
    string pinhole_file = (boost::format(pinhole_file_fmt) % i).str();
    if (!fs::exists(pinhole_file)) {
      continue;
    }
 
    OrbitFootprint o(pinhole_file, Vector2i(5725, 5725), moon_rad);
    renderbox.grow(o.bounding_box());
    orbits.push_back(o);
  }

  int renderlevel = 6;
  // Resolution: size of workspace, in tiles
  double resolution = 1 << renderlevel;
  Matrix3x3 transform;
  transform(0,0) = 360.0 / resolution;
  transform(0,2) = -180.0;
  transform(1,1) = -360.0 / resolution;
  transform(1,2) = 180.0;
  transform(2,2) = 1;
  transform = inverse(transform);

  BBox2 renderbox_trans;
  renderbox_trans.grow(dehom(transform * hom(renderbox.min())));
  renderbox_trans.grow(dehom(transform * hom(renderbox.max())));
 
  BBox2i tile_renderbox(floor(renderbox_trans.min()), ceil(renderbox_trans.max()));

  cout << tile_renderbox << endl;
  cout << tile_renderbox.width() << " " << tile_renderbox.height() << endl;

  return 0;
}
