/// \file FootprintProjector.h
///
/// Orbital Image Footprint Projector Class
///
/// TODO: Write me!
///

#ifndef __MVP_ORBITALIMAGE_FOOTPRINT_H__
#define __MVP_ORBITALIMAGE_FOOTPRINT_H__

#include <mvp/Geometry/ConvexPolygon.h>

#include <vw/Camera/CameraModel.h>
#include <vw/Cartography/Datum.h>

namespace mvp {
namespace geometry {

class FootprintProjector {
  boost::shared_ptr<vw::camera::CameraModel> m_camera;
  vw::cartography::Datum m_datum;

  public:
    FootprintProjector(boost::shared_ptr<vw::camera::CameraModel> camera, vw::cartography::Datum const& datum) :
      m_camera(camera), m_datum(datum) {}

    vw::Vector2 forward(vw::Vector2 const& lonlat, double alt);

    vw::Vector2 backward(vw::Vector2 const& px, double alt);

    ConvexPolygon forward(ConvexPolygon const& poly, vw::Vector2 const& alt_limits);

    ConvexPolygon backward(ConvexPolygon const& poly, vw::Vector2 const& alt_limits);
};

}} // namespace geometry, mvp

#endif
