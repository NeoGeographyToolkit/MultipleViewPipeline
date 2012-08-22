#include <mvp/OrbitalImage/FootprintCollection.h>

#include <boost/foreach.hpp>

namespace mvp {
namespace orbitalimage {

void FootprintCollection::push_back_pattern(std::string const& image_pattern, std::string const& camera_pattern) {
  std::cout << "Add image pattern!" << std::endl;
}

int FootprintCollection::equal_resolution_level() const {
  int result = std::numeric_limits<int>::max();

  BOOST_FOREACH(Footprint const& fp, *this) {
    result = std::min(result, fp.equal_resolution_level());
  }

  return result;
}

int FootprintCollection::equal_density_level(int tile_size) const {
  int result = 0;

  BOOST_FOREACH(Footprint const& fp, *this) {
    result = std::max(result, fp.equal_density_level(tile_size));
  }

  return result;
}

vw::BBox2 FootprintCollection::lonlat_bbox() const {
  vw::BBox2 result;

  BOOST_FOREACH(Footprint const& fp, *this) {
    result.grow(fp.bounding_box());
  }

  return result;
}

std::vector<pipeline::OrbitalImage> FootprintCollection::images_in_region(vw::BBox2 const& lonlat_bbox) const {
  std::vector<pipeline::OrbitalImage> result;

  BOOST_FOREACH(Footprint const& fp, *this) {
    if (fp.intersects(lonlat_bbox)) {
      result.push_back(fp.orbital_image());
    } 
  }

  return result;
}

}} // namespace orbitalimage,mvp
