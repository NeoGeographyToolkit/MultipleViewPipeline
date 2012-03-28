#include <mvp/MVPTileSeeder.h>
#include <mvp/MVPAlgorithm.h>

#include <vw/Cartography/SimplePointImageManipulation.h> // lon_lat_radius_to_xyz

namespace mvp {

bool MVPTileSeederDumb::init() {
  VW_ASSERT(m_result.georef.datum().semi_major_axis() == m_result.georef.datum().semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 
  
  using namespace vw;

  double cntr_pt = m_result.tile_size / 2.0;

  Vector2 cntr_lonlat = m_result.georef.pixel_to_lonlat(Vector2(cntr_pt, cntr_pt));

  m_seed.alt = (m_user_settings.alt_min() + m_user_settings.alt_max()) / 2;
  m_seed.orientation = cartography::lon_lat_radius_to_xyz(Vector3(cntr_lonlat[0], cntr_lonlat[1], 1)); // Assume not spheroid for this call
  m_seed.windows = Vector3(m_result.tile_size, m_result.tile_size, m_user_settings.seed_window_smooth_size());
  m_seed.windows /= m_user_settings.gauss_divisor();

  MVPAlgorithmOptions opts;
  opts.set_alt_range((m_user_settings.alt_max() - m_user_settings.alt_min()) / 2);
  opts.set_max_iterations(m_user_settings.max_iterations());
  opts.set_gauss_divisor(m_user_settings.gauss_divisor());

  MVPPixelResult result((*m_algorithm)(m_seed, vw::cartography::crop(m_result.georef, cntr_pt, cntr_pt), opts));

  if (result.converged) {
    m_seed = result;
    m_seed.windows = Vector3(m_user_settings.window_size(), m_user_settings.window_size(), m_user_settings.window_smooth_size());
    m_seed.windows /= m_user_settings.gauss_divisor();
    return true;
  } else {
    return false;
  }
}

MVPAlgorithmVar MVPTileSeederDumb::seed(int col, int row) const {
  return m_seed;
}

MVPPixelResult MVPTileSeederDumb::update(int col, int row, MVPAlgorithmVar const& seed) {
  MVPAlgorithmOptions opts;
  opts.set_alt_range(m_user_settings.alt_search_range());
  opts.set_max_iterations(m_user_settings.max_iterations());
  opts.set_gauss_divisor(m_user_settings.gauss_divisor());

  MVPPixelResult px_result((*m_algorithm)(seed, vw::cartography::crop(m_result.georef, col, row), opts));
  m_result.update(col, row, px_result);
  return px_result;
}

} // namespace mvp
