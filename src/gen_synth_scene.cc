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

// TODO: Rather than saving normalized images by having to go and
// recalculate everyting, it would be better to load the floating
// pt data and normalize that.

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
    write_georef_image(output_folder + "/initial-DEM.tif", out, georef);
    write_georef_image(output_folder + "/initial-DEM.norm.tif", 
                       pixel_cast_rescale<uint8>(out), georef);
  }

  // Create ground-DEM.tif
  Vector4 dem_ground_plane = gen_plane(georef, -2605, 888, dem_width, dem_height);
  {
    ImageViewRef<float32> out = pixel_cast<float32>(plane_dem_view(georef,
                                dem_ground_plane, dem_width, dem_height));
    write_georef_image(output_folder + "/ground-DEM.tif", out, georef);
    write_georef_image(output_folder + "/ground-DEM.norm.tif", 
                       pixel_cast_rescale<uint8>(out), georef);
  }

  // Create ground-DRG.tif
  {
    ImageViewRef<float32> out = pixel_cast<float32>(gaussian_filter(uniform_noise_view(
                                gen, dem_width, dem_height), 1.5));
    // Don't block write. If we block write, threading
    // will make the texture different every runtime
    write_georef_image(output_folder + "/ground-DRG.tif", out, georef, false);
    write_georef_image(output_folder + "/ground-DRG.norm.tif",
                       pixel_cast_rescale<uint8>(out), georef, false);
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
     
      write_orbit_image(output_folder + "/" + ss.str() + ".tif", out);
      write_orbit_image(output_folder + "/" + ss.str() + ".norm.tif",
                        pixel_cast_rescale<uint8>(out));
    }
    camera_list[i].write(output_folder + "/" + ss.str() + ".pinhole");
  }

  return 0;
}
