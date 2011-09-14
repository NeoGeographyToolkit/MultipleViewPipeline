#include <iostream>

#include <vw/Image.h>
#include <vw/Cartography.h>
#include <vw/FileIO.h>
#include <vw/Camera.h>

#include <vw/Plate/PlateFile.h>

#include "orbitfoot.h"

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;


using namespace vw;

ImageView<PixelRGBA<uint8> > render_tile(int i, int j, int renderlevel, int tile_size, std::vector<OrbitFootprint> const& affected, double rad)
{
  ImageView<PixelRGBA<uint8> > tile(tile_size, tile_size);
  if (affected.size() == 0) return tile;

  BBox2 cam_bounds(0, 0, 5725, 5725);

  std::vector<camera::PinholeModel> cams;

  BOOST_FOREACH(OrbitFootprint o, affected) {
    cams.push_back(o.camfile());
  }

  // Resolution: size of workspace, in pixels
  double resolution = tile_size << renderlevel;
  Matrix3x3 transform;
  transform(0,0) = 360.0 / resolution;
  transform(0,2) = -180.0;
  transform(1,1) = -360.0 / resolution;
  transform(1,2) = 180.0;
  transform(2,2) = 1;

  Vector2 tileoffset(tile_size*i, tile_size*j);

  //TODO: instead, get lonlat BBox from tile's col, row, and lvl
  //      and then make a mapping from 0-tile_size => lonlat.min-lonlat.max
  //
  // Vector3 llr(0, 0, rad);
  // subvector(llr, 0, 2) = Vector2(i,j) * (lonlat.max - lonlat.min) / tile_size + lonlat.min

  for (int i = 0; i < tile_size; i++) {
    for (int j = 0; j < tile_size; j++) {
      Vector3 llr = transform * hom(Vector2(i, j) + tileoffset);
      llr[2] = rad;

      Vector3 xyz = cartography::lon_lat_radius_to_xyz(llr);

      int numovers = 0;
      BOOST_FOREACH(camera::PinholeModel cam, cams) {
        Vector2 orb_px = cam.point_to_pixel(xyz);
        if (cam_bounds.contains(orb_px)) {
          numovers++;
        }
      }
      if (numovers > 0) {
        int color = int(255.0 * numovers / 20);
        color = std::min(color, 255);
        tile(i, j) = PixelRGBA<vw::uint8>(color, color, color, 255);
      }
    }
  }
  return tile;
}

int main()
{
  using std::cout;
  using std::endl;
  using std::string;

  string pinhole_file_fmt = "/byss/khusmann/data/AS15_cameras/AS15-M-%04d.lev1.pinhole";
  double moon_rad = 1737400.0;
  int tile_size = 256;

  std::vector<OrbitFootprint> orbits;
  BBox2 renderbox;

//  for (int i = 73; i < 2492; i++) {
  for (int i = 73; i < 600; i++) {
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

  boost::shared_ptr<platefile::PlateFile> pf(new platefile::PlateFile("output.plate", "equi", "desc", tile_size, "tif", VW_PIXEL_RGBA, VW_CHANNEL_UINT8));
  platefile::Transaction tid = pf->transaction_request("tdesc", -1);

  for (int i = tile_renderbox.min().x(); i < tile_renderbox.max().x(); i++) {
    for (int j = tile_renderbox.min().y(); j < tile_renderbox.max().y(); j++) {
      std::vector<OrbitFootprint> affected;

      BOOST_FOREACH(OrbitFootprint o, orbits) {
        if (o.intersects(i, j, renderlevel)) {
            affected.push_back(o);
        }
      }

      ImageView<PixelRGBA<uint8> > rendered_tile;
      
      cout << "Rendering tile: (" << i << ", " << j << ") @" << renderlevel << " affected: " << affected.size() << endl;
      rendered_tile = render_tile(i, j, renderlevel, tile_size, affected, moon_rad);

      // Add to platefile
      pf->write_request();
      pf->write_update(rendered_tile, i, j, renderlevel, tid);
      pf->sync();
      pf->write_complete();
    }
  }

  return 0;
}
