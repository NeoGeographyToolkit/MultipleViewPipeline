#include <iostream>

#include <vw/Image.h>
#include <vw/Cartography.h>
#include <vw/FileIO.h>
#include <vw/Camera.h>

#include <boost/format.hpp>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

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

// Metadata about a specific orbital image
// Right now just holds the path to the camera
// model
class OrbitalImage
{
  std::string m_camera_model;
  public:
    OrbitalImage(std::string const& camera_model) : 
      m_camera_model(camera_model) 
    {}

    std::string camera_model() {return m_camera_model;}
};

// For a given tile, we need to find which orbital
// images possibly overlap. This tree performs that
// lookup
class TileOrbitLookupTree
{
  private:
    struct TreeNode
    {
      int col, row, lvl;
      // Two bits determine the child, organized col|row
      // So 00 is the upper left corner and 11 is the lower right
      boost::shared_ptr<TreeNode> child[4];
      std::vector<boost::shared_ptr<OrbitalImage> > images;

      TreeNode(int c, int r, int l) : col(c), row(r), lvl(l) {}
    };

    void insert_node(int col, int row, int lvl, boost::shared_ptr<OrbitalImage> im) {
      for (int i = lvl; i >= 0; i--) {
        
      }

    }

    boost::shared_ptr<TreeNode> m_root;

  public:
    TileOrbitLookupTree() :
      m_root(new TreeNode(0, 0, 0)) {}

    void add_orbital_image(boost::shared_ptr<OrbitalImage> im) {
      // find level for image
      // create a bbox for the image at that lvl
      // 
      // dig for that TreeNode, append to images field
    }
};



int main()
{
  using namespace std;

  string pinhole_file_fmt = "/byss/khusmann/data/AS15_cameras/AS15-M-%04d.lev1.pinhole";

  TileOrbitLookupTree tolt;

  for (int i = 73; i < 2492; i++) {
    string pinhole_file = (boost::format(pinhole_file_fmt) % i).str();
    if (!fs::exists(pinhole_file)) {
      continue;
    }

    boost::shared_ptr<OrbitalImage> im(new OrbitalImage(pinhole_file));

    vw::camera::PinholeModel cam(pinhole_file);
    vw::BBox2 fp_bbox;
    vw::Vector2i img_size(5725, 5725);
    vw::Vector2i tile_size(256, 256);

    vw::cartography::Datum datum;
    datum.set_well_known_datum("D_MOON");
    double moon_rad = datum.radius(0, 0);

    fp_bbox.grow(backproj_px(cam, vw::Vector2i(0, 0), moon_rad));
    fp_bbox.grow(backproj_px(cam, vw::Vector2i(img_size.x(), 0), moon_rad));
    fp_bbox.grow(backproj_px(cam, vw::Vector2i(0, img_size.y()), moon_rad));
    fp_bbox.grow(backproj_px(cam, vw::Vector2i(img_size.x(), img_size.y()), moon_rad));

    double ilvl_width_px = 360.0 * tile_size.x() / fp_bbox.width();
    double ilvl_height_px = 360.0 * tile_size.y() / fp_bbox.height();
    double ilvl_px = std::max(std::max(ilvl_width_px, ilvl_height_px), 256.0);

    // Round resolution to the nearest power of two. The base of
    // the pyramid is 2^8 (256 pixels)
    int level = static_cast<int>(ceil(log(ilvl_px) / log(2))) - 8;

    double lvl_res = (8 << level) / 360.0;

    vw::BBox2 fp_px_bbox = fp_bbox * lvl_res;

    tolt.add_orbital_image(im);
  }


  return 0;
}
