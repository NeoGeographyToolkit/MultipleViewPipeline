#include <iostream>
#include <vw/Image.h>
#include <vw/FileIO.h>
#include <vw/Camera.h>
#include <vw/Cartography.h>
#include <vw/Math.h>
#include <string>
#include <vw/Plate/PlateFile.h>
#include <vw/Plate/PlateManager.h>


vw::Vector2 backproj_px(vw::camera::PinholeModel const& cam, vw::Vector2 const& px, double sphere_rad)
{
  vw::Vector3 dir = cam.pixel_to_vector(px);
  vw::Vector3 cntr = cam.camera_center();

  double a = dot_prod(dir, dir);
  double b = 2 * dot_prod(cntr, dir);
  double c = dot_prod(cntr, cntr) - sphere_rad * sphere_rad;

  double t = (-b - sqrt(b*b-4*a*c))/2/a;

  vw::Vector3 isect = cntr + t * dir;

  vw::Vector3 llr = vw::cartography::xyz_to_lon_lat_radius(isect);

  return vw::math::subvector(llr, 0, 2);
}

int main() 
{
  std::string camfile = "/byss/khusmann/data/AS15_cameras/AS15-M-0996.lev1.pinhole";
  vw::Vector2i img_size(5725, 5725);
  vw::Vector2i tile_size(512, 512);

  vw::cartography::Datum datum;
  datum.set_well_known_datum("D_MOON");
  double moon_rad = datum.radius(0, 0);

  vw::camera::PinholeModel cam(camfile);

  vw::BBox2 fp_bbox;
  fp_bbox.grow(backproj_px(cam, vw::Vector2i(0, 0), moon_rad));
  fp_bbox.grow(backproj_px(cam, vw::Vector2i(img_size.x(), 0), moon_rad));
  fp_bbox.grow(backproj_px(cam, vw::Vector2i(0, img_size.y()), moon_rad));
  fp_bbox.grow(backproj_px(cam, vw::Vector2i(img_size.x(), img_size.y()), moon_rad));

  double x_res = fp_bbox.width() / tile_size.x();
  double y_res = fp_bbox.height() / tile_size.y();
  double res = std::max(x_res, y_res);

  vw::Matrix3x3 trans;
  trans.set_identity();
  trans(0, 0) = res;
  trans(1, 1) = -res;
  trans(0, 2) = fp_bbox.min().x();
  trans(1, 2) = fp_bbox.max().y();

  std::cout << trans << std::endl;
  
  vw::cartography::GeoReference georef(datum, trans);

  vw::ImageView<vw::PixelRGBA<vw::uint8> > img(tile_size.x(), tile_size.y());


  vw::BBox2i orb_bbox(vw::Vector2i(0, 0), img_size);

  for (int r = 0; r < img.rows(); r++) {
    for (int c = 0; c < img.cols(); c++) {
      vw::Vector3 llr;
      vw::math::subvector(llr, 0, 2) = georef.pixel_to_lonlat(vw::Vector2(c, r));
      llr[2] = moon_rad;
      vw::Vector3 xyz = vw::cartography::lon_lat_radius_to_xyz(llr);
      vw::Vector2 orb_px = cam.point_to_pixel(xyz);
      if (orb_bbox.contains(orb_px)) {
        img(c, r) = vw::PixelRGBA<vw::uint8>(0, 0, 255, 255);
      }
    }
  }
  write_image("test.png", img);

  boost::shared_ptr<vw::platefile::PlateFile> pf(new vw::platefile::PlateFile("blah.plate", "equi", "", 256, "tif", img.pixel_format(), img.channel_type()));
  vw::platefile::PlateManager<vw::PixelRGBA<vw::uint8> > *pm = vw::platefile::PlateManager<vw::PixelRGBA<vw::uint8> >::make("equi", pf);

  pm->insert(vw::DiskImageView<vw::PixelRGBA<vw::uint8> >("test.png"), "desc", -1, georef, false, true, vw::TerminalProgressCallback("blah", "Processing: "));
  
  delete pm;

  return 0;
}
