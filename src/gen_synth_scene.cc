// __BEGIN_LICENSE__
// Copyright (C) 2006-2010 United States Government as represented by
// the Administrator of the National Aeronautics and Space Administration.
// All Rights Reserved.
// __END_LICENSE__

#include "gen_synth_scene.h"

using std::cout;
using std::endl;
using std::string;

using namespace vw;
using namespace vw::camera;
using namespace vw::cartography;

int main( int argc, char *argv[] ) {
  boost::rand48 gen;
  const int dem_width = 1024, dem_height = 1024;
  string output_folder = ".";
  GeoReference georef = gen_dem_georef();
  std::vector<PinholeModel> camera_list = gen_camera_list();

  if (argc > 1) {
    output_folder = argv[1];
  }

  // Create initial-DEM.tif
  Vector4 dem_initial_plane = gen_plane(georef, -2605, dem_width, dem_height);
  {
    ImageViewRef<float32> out = pixel_cast<float32>(plane_dem_view(georef,
                                dem_initial_plane, dem_width, dem_height));
    DiskImageResourceGDAL rsrc(output_folder + "/initial-DEM.tif", out.format(),
                               Vector2i(256, 256));
    write_georeference(rsrc, georef);
    block_write_image(rsrc, out,
      TerminalProgressCallback("vw", "initial-DEM.tif: "));
  }

  // Create ground-DEM.tif
  Vector4 dem_ground_plane = gen_plane(georef, -2605, 888, dem_width, dem_height);
  {
    ImageViewRef<float32> out = pixel_cast<float32>(plane_dem_view(georef,
                                dem_ground_plane, dem_width, dem_height));
    DiskImageResourceGDAL rsrc(output_folder + "/ground-DEM.tif", out.format(),
                               Vector2i(256, 256));
    write_georeference(rsrc, georef);
    block_write_image(rsrc, out,
      TerminalProgressCallback("vw", "ground-DEM.tif: "));
  }

  // Create ground-DRG.tif
  {
    ImageViewRef<float32> out = pixel_cast<float32>(gaussian_filter(uniform_noise_view(
                                gen, dem_width, dem_height), 1.5));
    DiskImageResourceGDAL rsrc(output_folder + "/ground-DRG.tif", out.format(),
                               Vector2i(256, 256));
    write_georeference(rsrc, georef);
    // Don't block write. If we block write, threading
    // will make the texture different every runtime
    write_image(rsrc, out,
      TerminalProgressCallback("vw", "ground-DRG.tif: "));
  }

  // Create Orbital Images
  DiskImageView<float32> drg_ground(output_folder + "/ground-DRG.tif");
  for (unsigned i = 0; i < camera_list.size(); i++) {
    std::stringstream ss;
    ss << i;
    {
      ImageViewRef<float32> out = backproject_plane(interpolate(drg_ground, 
        BilinearInterpolation(), ZeroEdgeExtension()) , georef, camera_list[i],
        dem_ground_plane, 1800, 1800);
      DiskImageResourceGDAL rsrc(output_folder + "/" + ss.str() + ".tif", out.format(),
                                 Vector2i(256, 256));
      block_write_image(rsrc, out, TerminalProgressCallback("vw", ss.str() + ".tif: "));
    }
    camera_list[i].write(output_folder + "/" + ss.str() + ".pinhole");
  }

  return 0;
}
