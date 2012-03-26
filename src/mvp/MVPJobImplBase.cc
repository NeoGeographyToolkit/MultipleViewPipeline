#include <mvp/MVPJobImplBase.h>

#include <vw/Cartography/SimplePointImageManipulation.h>

namespace mvp {

MVPPixelResult MVPJobImplBase::generate_seed() const {
  double seed_col = m_tile_size / 2.0;
  double seed_row = m_tile_size / 2.0;

  vw::Vector2 seed_lonlat = m_georef.pixel_to_lonlat(vw::Vector2(seed_col, seed_row));

  MVPAlgorithmVar pre_seed;
  pre_seed.alt = (m_settings.alt_min() + m_settings.alt_max()) / 2;
  VW_ASSERT(m_georef.datum().semi_major_axis() == m_georef.datum().semi_minor_axis(), vw::NoImplErr() << "Spheroid datums not supported"); 
  // TODO: The following calculation assumes spherical datum
  pre_seed.orientation = vw::cartography::lon_lat_radius_to_xyz(vw::Vector3(seed_lonlat[0], seed_lonlat[1], 1));
  pre_seed.windows = vw::Vector3(m_tile_size, m_tile_size, m_settings.seed_window_smooth_size());

  MVPAlgorithmOptions options;
  options.set_alt_range((m_settings.alt_max() - m_settings.alt_min()) / 2);
  options.set_fix_orientation(true);
  options.set_fix_windows(true);
  options.set_max_iterations(m_settings.max_iterations());
  options.set_gauss_divisor(m_settings.gauss_divisor());

  return process_pixel(pre_seed, seed_col, seed_row, options);
}

MVPTileResult MVPJobImplBase::process_tile(vw::ProgressCallback const& progress) const {
  MVPTileResult tile_result(m_georef, m_tile_size);

  MVPPixelResult seed(generate_seed());

  if (!seed.converged) {
    return tile_result;
  }

  MVPAlgorithmOptions options;
  options.set_alt_range(m_settings.alt_search_range());
  options.set_fix_orientation(true); 
  options.set_fix_windows(true);
  options.set_max_iterations(m_settings.max_iterations());
  options.set_gauss_divisor(m_settings.gauss_divisor());

  // Set window size
  seed.windows = vw::Vector3(m_settings.window_size(), m_settings.window_size(), m_settings.window_smooth_size());

  int curr_px_num = 0;
  int num_px_to_process = m_tile_size * m_tile_size;
  for (int col = 0; col < m_tile_size; col++) {
    for (int row = 0; row < m_tile_size; row++) {
      progress.report_fractional_progress(curr_px_num++, num_px_to_process);
      tile_result.update(col, row, process_pixel(seed, col, row, options));
    }
  }
  progress.report_finished();
  return tile_result;
}

} // namespace mvp
