#include <mvp/MVPJobImpl.h>

#include <vw/Cartography/SimplePointImageManipulation.h> // lon_lat_radius_to_xyz

namespace mvp {

MVPPixelResult MVPJobImpl::process_pixel(MVPAlgorithmVar const& seed, 
                                         vw::cartography::GeoReference const& georef, 
                                         MVPAlgorithmOptions const& options) const 
{
  using namespace vw;

  // TODO: MVP Algorithm implementation goes here...

  return MVPPixelResult();
}

MVPPixelResult MVPJobImplFootprint::process_pixel(MVPAlgorithmVar const& seed, 
                                                  vw::cartography::GeoReference const& georef, 
                                                  MVPAlgorithmOptions const& options) const 
{
  VW_ASSERT(georef.datum().semi_major_axis() == georef.datum().semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 
  using namespace vw;

  Vector2 ll = georef.pixel_to_lonlat(Vector2(0, 0));
  Vector3 llr(ll[0], ll[1], georef.datum().semi_major_axis());
  Vector3 xyz = cartography::lon_lat_radius_to_xyz(llr);

  int overlaps = 0;
  BOOST_FOREACH(OrbitalImageCrop const& o, m_crops) {
    Vector2 px = o.camera().point_to_pixel(xyz);
    if (bounding_box(o).contains(px)) {
      overlaps++;
    }
  }

  MVPAlgorithmVar result(overlaps, Vector3f(overlaps, overlaps, overlaps), Vector3f(overlaps, overlaps, overlaps));

  return MVPPixelResult(result, overlaps, overlaps > 0, overlaps);
}

} // namespace mvp

#if MVP_ENABLE_OCTAVE_SUPPORT
#include <octave/parse.h>

namespace mvp {

MVPPixelResult MVPJobImplOctave::process_pixel(MVPAlgorithmVar const& seed, 
                                               vw::cartography::GeoReference const& georef, 
                                               MVPAlgorithmOptions const& options) const 
{
  ::octave_value_list args;
  args.append(seed.to_octave());
  args.append(vw::octave::georef_to_octave(georef));
  args.append(m_octave_crops);
  args.append(vw::octave::protobuf_to_octave(&options));

  return MVPPixelResult(::feval(MVP_OCTAVE_ALGORITHM_FCN, args, 1));
}

MVPPixelResult MVPJobImplFootprintOctave::process_pixel(MVPAlgorithmVar const& seed, 
                                                        vw::cartography::GeoReference const& georef, 
                                                        MVPAlgorithmOptions const& options) const 
{
  ::octave_value_list args;
  args.append(seed.to_octave());
  args.append(vw::octave::georef_to_octave(georef));
  args.append(m_octave_crops);
  args.append(vw::octave::protobuf_to_octave(&options));

  return MVPPixelResult(::feval(MVP_OCTAVE_FOOTPRINT_FCN, args, 1));
}

} // namespace mvp
#endif

