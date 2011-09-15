#include <octave/oct.h>

#include <vw/Image.h>
#include <vw/FileIO.h>
#include <vw/Cartography.h>
#include <vw/Camera.h>

// TODO: Make a test for this too
Matrix pinhole_to_octave(vw::camera::PinholeModel const& vw_cam)
{
  Matrix oct_cam_mat(3, 4);

  for (int r = 0; r < 3; r++) {
    for (int c = 0; c < 4; c++) {
      oct_cam_mat(r, c) = vw_cam_mat(r, c);
    }
  }

  Matrix rebase = identity_matrix(3, 3);
  rebase(0, 2) = rebase(1, 2) = 1;
  oct_cam_mat = rebase * oct_cam_mat;

  return oct_cam_mat;
}

// TODO: Make a test for this function
template <class ViewT>
Matrix imageview_to_octave(vw::ImageViewBase<ViewT> const& vw_img) 
{
  typedef vw::ImageView<double> RasterT;

  // Rasterize image before copying to octave
  RasterT rast = vw::pixel_cast<double>(vw_img.impl());

  Matrix oct_img(rast.rows(), rast.cols());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      oct_img(row, col) = *cacc;
      cacc.next_col();
    }
    racc.next_row();
  }

  return oct_img;
}

vw::ImageView<double> octave_to_imageview(Matrix const& oct_img)
{
  typedef vw::ImageView<double> RasterT;

  RasterT rast(oct_img.cols(), oct_img.rows());

  typedef RasterT::pixel_accessor AccT;
  AccT racc = rast.origin();

  for(int row = 0; row < rast.rows(); row++) {
    AccT cacc = racc;
    for(int col = 0; col < rast.cols(); col++) {
      *cacc = oct_img(row, col);
      cacc.next_col();
    }
    racc.next_row();
  }

  return rast;
}

int main(int argc, char *argv[])
{
  vw::ImageView<vw::uint8> img;
  vw::read_image(img, argv[1]);

  Matrix mat = imageview_to_octave(img);
  vw::ImageView<vw::uint8> out = vw::pixel_cast<vw::uint8>(octave_to_imageview(mat));
  vw::write_image("out.png", out);
  /*
    Types of BBoxes:
    tile_bbox
    pixel_bbox
    lonlat_bbox



  OrbitalImage("pinhole", size/"img", parent_workspace))
  - equal_resolution_level()
  - equal_density_level()
  - intersects(lonlat_bbox)

  MVPWorkspace
  - given lvl, georef for entire workspace
  - given tile coords -> georef

  - to create lonlatBBox given tile coords: get tile georef then georef.pixel_to_lonlat_bbox(BBox2i(0, 0, tile_size.x(), tile_size.y()))
  - to create a pixelBBox given lonlat BBox: get world georef then georef.lonlat_to_pixel_bbox(lonlatBBox)

  Conversions done by MVPWorkspace (<- actually, should this be a separate class??)
  - lonlat coords -> pixel coords
  - tile coords -> pixel bbox -> lonlat bbox
  ^^ depends on tile size, so make member funcs of MVPWorkspace

  MVPWorkspace work(tile_size, Datum, "process", bounds)
  work.add_image("cam", size/"img")
  work.add_image_pattern("cam_pattern", size/"img_ptrn")
  cout << work.lonlat_bbox()
  cout << work.equal_resolution_level() << work.equal_density_level()
  cout << work.tile_bbox(eq_res_level) << work.tile_bbox(eq_dens_lvl)

  OrbitalImageCollection col = work.get_orbital_images(col, row, lvl)

  MVPJobRequest job = work.assemble_job(col, row, lvl)
  - tile_size, GEOREF, process, col, row, lvl, (camera_list, image_list, image_size_list)

  MVPSubproblem prob(job)
  - tile_size, georef, process, col, row, lvl, (camera_list, image_list, image_size_list, crop_camera_list, crop_image_list)
  - create protobuf for prob, so it can be written to a file too.
  prob.write("debug.prob")
  ImageView<double> result = prob.process()
  pf->write_update(result, prob.col, prob.row, prob.lvl, prob.tid)
  

  - Maybe, figure out the portions of the orbital images being used, then filter them
    down to the same resolution of the tile being worked on??? **** this could work!
  - When outputting subproblems, also include versions that super-impose the tile boundary on the orbital image
      

  // Two interesting levels: 
  //    where the pixel_density(image) == pixel_density(plate@level) (for most dense image)
  //    where the resolution(image) == resolution(plate_tile@level) (for highest resolution)

  Requests to Zach:
  - Bresham line class in VW
  - Georef: pixel_to_point_bbox, point_to_pixel_bbox, point_to_lonlat_bbox, point_to_lonlat_bbox, lonlat_to_point_bbox
  - Protobuf for georefs

  MVPWorkspace(Datum("D_MOON"), 256, Vector2())

  TODO:
  1) get protobuf to work in cmake
  2) write protobuf for MVPOperationDesc and MVPJobRequest
  3) implement MVPWorkspace, OrbitalImage & tests
  4) implement MVPTileProcessor
  5) design MVPOperation

  MVPAlgorithm(MVPAlgorithmDesc): Given an x,y, a georeference, a set of images/cameramodels, seed => compute post height

  Types of Algorithms: MVP algorithm. orthoimage (albedo), errors, footprint

  function MVPAlgorithmFootprint(

  Reflection for automatically registering stuff: http://stackoverflow.com/questions/582331/is-there-a-way-to-instantiate-objects-from-a-string-holding-their-class-name
  Protobuf unions: http://code.google.com/apis/protocolbuffers/docs/techniques.html
  Gearman start: http://toys.lerdorf.com/archives/51-Playing-with-Gearman.html
  */
}
