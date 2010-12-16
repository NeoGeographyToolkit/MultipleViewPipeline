#ifndef __ASP_MULTIVIEWTK_GENSYNTHSCENE_H__
#define __ASP_MULTIVIEWTK_GENSYNTHSCENE_H__

#include <boost/foreach.hpp>
#include <boost/random/linear_congruential.hpp>

#include <vw/FileIO.h>
#include <vw/Image.h>
#include <vw/Cartography.h>
#include <vw/Math.h>
#include <vw/Camera.h>

namespace vw {
namespace cartography {

Vector3 lonlat_to_normal(Vector2 const& p, bool east_positive=true) {
  // Convert from lon, lat to an xyz direction vector pointing
  // in the r direction
  //
  // East positive:
  // x = cos(latitude) * cos(longitude)
  // y = cos(latitude) * sin(longitude)
  // z = sin(latitude)

  if (east_positive) {
    return Vector3(cos(p[1] * M_PI/180.0) * cos(p[0] * M_PI/180.0),
                   cos(p[1] * M_PI/180.0) * sin(p[0] * M_PI/180.0),
                   sin(p[1] * M_PI/180.0));
  } else {
    return Vector3(cos(p[1] * M_PI/180.0) * cos(-p[0] * M_PI/180.0),
                   cos(p[1] * M_PI/180.0) * sin(-p[0] * M_PI/180.0),
                   sin(p[1] * M_PI/180.0));
  }
}

struct PlaneDEMFunctor {
  typedef double result_type;
  GeoReference m_georef;
  Vector4 m_plane;
  PlaneDEMFunctor(GeoReference const& georef, Vector4 const& plane) : 
    m_georef(georef), m_plane(plane) {}
  result_type operator()(double i, double j, int32 /*p*/) const {
    // Assumes east is positive longitude for the georef...
    Vector2 lonlat = m_georef.pixel_to_lonlat(Vector2(i, j));
    double r = m_plane[3] / dot_prod(Vector3(m_plane[0], m_plane[1], m_plane[2]),
                                     lonlat_to_normal(lonlat));
    return r - m_georef.datum().radius(lonlat[0], lonlat[1]);
  }
};

inline PerPixelIndexView<PlaneDEMFunctor>
plane_dem_view(GeoReference const& georef, Vector4 const& plane,
               int32 cols, int32 rows) {
  typedef PerPixelIndexView<PlaneDEMFunctor> result_type;
  return result_type(PlaneDEMFunctor(georef, plane), cols, rows);
}

inline PerPixelIndexView<PlaneDEMFunctor>
plane_dem_view(GeoReference const& georef, Vector3 const& normal, 
               Vector3 const& p, int32 cols, int32 rows) {
  VW_LINE_ASSERT(fabs(norm_2(normal) - 1) < 0.001);
  Vector4 plane(normal[0], normal[1], normal[2], dot_prod(normal, p));
  return plane_dem_view(georef, normal, plane, cols, rows);
}

struct LonLatIndexFunctor {
  typedef Vector2 result_type;
  GeoReference m_georef;
  LonLatIndexFunctor(GeoReference const& georef) : m_georef(georef) {}
  result_type operator()(double i, double j, int32 /*p*/) {
    return m_georef.pixel_to_lonlat(Vector2(i, j));
  }
};

inline PerPixelIndexView<LonLatIndexFunctor>
lonlat_index_view(GeoReference const& georef, 
                  int32 cols, int32 rows, int32 planes = 1) {
  typedef PerPixelIndexView<LonLatIndexFunctor> result_type;
  return result_type(LonLatIndexFunctor(georef), cols, rows, planes);
}

template <class ImageT>
struct BackprojectPlaneFunctor {
  typedef typename ImageT::result_type result_type;
  ImageT m_image;
  GeoReference m_georef;
  camera::PinholeModel m_camera;
  Vector4 m_plane;
  BackprojectPlaneFunctor(ImageT const& image, GeoReference const& georef,
                          camera::PinholeModel const& camera,
                          Vector4 const& plane) :
    m_image(image), m_georef(georef), m_camera(camera), m_plane(plane) {}
  result_type operator()(double i, double j, int32 /*p*/) const {
    Vector3 camera_center = m_camera.camera_center();
    Vector3 ray_dir = m_camera.pixel_to_vector(Vector2(i, j));
    Vector3 normal(m_plane[0], m_plane[1], m_plane[2]);
    double t = (m_plane[3] - dot_prod(normal, camera_center)) /
               dot_prod(normal, ray_dir);
    Vector3 isect = camera_center + t * ray_dir;
    Vector3 isect_llr = xyz_to_lon_lat_radius(isect);
    Vector2 pixel = m_georef.lonlat_to_pixel(Vector2(isect_llr[0], isect_llr[1]));
    return m_image(pixel.x(), pixel.y());
  }
};

template <class ImageT>
inline PerPixelIndexView<BackprojectPlaneFunctor<ImageT> >
backproject_plane(ImageViewBase<ImageT> const& image, GeoReference const& georef,
                  camera::PinholeModel const& camera, Vector4 const& plane,
                  int32 cols, int32 rows) {
  typedef PerPixelIndexView<BackprojectPlaneFunctor<ImageT> > result_type;
  return result_type(BackprojectPlaneFunctor<ImageT>(
    image.impl(), georef, camera, plane), cols, rows);
}

}} // namespace cartography namespace vw

vw::cartography::GeoReference gen_dem_georef(int dem_width, int dem_height) {
  using namespace vw;
  using namespace vw::cartography;
  // 1024x1024 DEM georef
  // Original DEM mean: -2604.93 stddev:888.459
  double cntr_lon = 56.4052159036408;
  double cntr_lat = 9.51069160455920;
  double deg_per_px = 0.00133653359715;

  double georef_affine_data[] = { deg_per_px,         0.0, cntr_lon - deg_per_px * (dem_width / 2.0),
                                         0.0, -deg_per_px, cntr_lat + deg_per_px * (dem_height / 2.0),
                                         0.0,         0.0, 1.0 };

  return GeoReference(Datum("D_MOON"), MatrixProxy<double>(georef_affine_data, 3, 3));
}

std::vector<vw::camera::PinholeModel> gen_camera_list(int orbit_width, int orbit_height) {
  using namespace vw;
  using namespace vw::camera;
  std::vector<PinholeModel> camera_list;

  camera_list.push_back(PinholeModel(
    Vector3(966089.223462, 1557938.52831, 282405.060851),
    Quat(-0.0470851319085,0.358002222657,0.665010992829,-0.653741369612).rotation_matrix(),
    3802.7, 3802.7, 1851.375 + orbit_width / 2, -90.5 + orbit_height / 2,
    Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), NullLensDistortion()
    ));

  camera_list.push_back(PinholeModel(
    Vector3(996045.240112, 1535573.08189, 299032.543447),
    Quat(-0.0540368023579, 0.364928919676, 0.665264280335, -0.649099641723).rotation_matrix(),
    3802.7, 3802.7, 649.875 + orbit_width / 2, -86.124 + orbit_height / 2,
    Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), NullLensDistortion()
    ));

  camera_list.push_back(PinholeModel(
    Vector3(1025532.88151, 1512446.05054, 315515.076309),
    Quat(-0.0588100780917, 0.371592737959, 0.667265027691, -0.642820032849).rotation_matrix(),
    3802.7, 3802.7, -577.5 + orbit_width / 2, -59 + orbit_height / 2,
    Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), NullLensDistortion()
    ));

  camera_list.push_back(PinholeModel(
    Vector3(1054501.51827, 1488593.63697, 331834.757985),
    Quat(-0.0639146875174, 0.378441350432, 0.667943479689, -0.637611609793).rotation_matrix(),
    3802.7, 3802.7, -1795 + orbit_width / 2, -40.5 + orbit_height / 2,
    Vector3(1, 0, 0), Vector3(0, 1, 0), Vector3(0, 0, 1), NullLensDistortion()
    ));

  return camera_list;
}

// Generate a plane with height cntr_height at the center
// of the DEM, and a normal pointing in the radial direction
// out of the center of the DEM
vw::Vector4 gen_plane(vw::cartography::GeoReference const& georef, 
                      double cntr_height, vw::int32 cols, vw::int32 rows)
{
  using namespace vw;
  Vector2 dem_cntr(cols / 2.0, rows / 2.0);
  Vector2 dem_cntr_ll(georef.pixel_to_lonlat(dem_cntr));
  Vector3 dem_cntr_llr(dem_cntr_ll[0], dem_cntr_ll[1],
                       cntr_height + georef.datum().radius(dem_cntr_ll[0], dem_cntr_ll[1]));
  Vector3 dem_cntr_xyz(vw::cartography::lon_lat_radius_to_xyz(dem_cntr_llr));
  Vector3 normal(vw::cartography::lonlat_to_normal(dem_cntr_ll));

  return Vector4(normal[0], normal[1], normal[2], dot_prod(normal, dem_cntr_xyz));
}

// Generate a plane with height cntr_height at the center
// of the DEM, height cntr_height + delta_height at the
// upper left hand corner of the DEM, and height cntr_height
// at the upper right hand corner of the DEM.
vw::Vector4 gen_plane(vw::cartography::GeoReference const& georef, 
                      double cntr_height, double delta_height,
                      vw::int32 cols, vw::int32 rows)
{
  using namespace vw;
  Vector2 dem_cntr(cols / 2.0, rows / 2.0);
  Vector2 dem_cntr_ll(georef.pixel_to_lonlat(dem_cntr));
  Vector3 dem_cntr_llr(dem_cntr_ll[0], dem_cntr_ll[1],
                       cntr_height + georef.datum().radius(dem_cntr_ll[0], dem_cntr_ll[1]));
  Vector3 dem_cntr_xyz(vw::cartography::lon_lat_radius_to_xyz(dem_cntr_llr));

  Vector2 dem_ul(0.0, 0.0);
  Vector2 dem_ul_ll(georef.pixel_to_lonlat(dem_ul));
  Vector3 dem_ul_llr(dem_ul_ll[0], dem_ul_ll[1],
                     cntr_height + delta_height + 
                     georef.datum().radius(dem_ul_ll[0], dem_ul_ll[1]));
  Vector3 dem_ul_xyz(vw::cartography::lon_lat_radius_to_xyz(dem_ul_llr));

  Vector2 dem_ur(cols, 0.0);
  Vector2 dem_ur_ll(georef.pixel_to_lonlat(dem_ur));
  Vector3 dem_ur_llr(dem_ur_ll[0], dem_ur_ll[1],
                     cntr_height + georef.datum().radius(dem_ur_ll[0], dem_ur_ll[1]));
  Vector3 dem_ur_xyz(vw::cartography::lon_lat_radius_to_xyz(dem_ur_llr));

  Vector3 normal(normalize(cross_prod(dem_ur_xyz - dem_cntr_xyz, dem_ul_xyz - dem_cntr_xyz)));

  return Vector4(normal[0], normal[1], normal[2], dot_prod(normal, dem_cntr_xyz));
}

template <class ImageT>
void write_georef_image(std::string const& filename, 
                        vw::ImageViewBase<ImageT> const& img, 
                        vw::cartography::GeoReference const& georef, 
                        bool bwrite = true) {
  // TODO: We really need a helper function in vw to do something like this...
  using namespace vw;
  DiskImageResourceGDAL rsrc(filename, img.impl().format(), Vector2i(256, 256));
  write_georeference(rsrc, georef);
  if (bwrite) {
    block_write_image(rsrc, img, TerminalProgressCallback("vw", filename + ": "));
  } else {
    write_image(rsrc, img, TerminalProgressCallback("vw", filename + ": "));
  }
}

template <class ImageT>
void write_orbit_image(std::string const& filename, 
                       vw::ImageViewBase<ImageT> const& img) {
  using namespace vw;
  DiskImageResourceGDAL rsrc(filename, img.format(), Vector2i(256, 256));
  block_write_image(rsrc, img, TerminalProgressCallback("vw", filename + ": "));
}

#endif
