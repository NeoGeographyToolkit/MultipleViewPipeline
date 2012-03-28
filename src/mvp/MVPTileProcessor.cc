#include <mvp/MVPTileProcessor.h>
#include <mvp/MVPTileSeeder.h>

namespace mvp {

MVPTileResult MVPTileProcessorDumb::operator()(vw::ProgressCallback const& progress) const {
  if (!m_seeder->init()) {
    progress.report_finished();
    return m_seeder->result();    
  }

  int tile_size = m_seeder->result().tile_size;

  int curr_px_num = 0;
  int num_px_to_process = tile_size * tile_size;
  for (int i = 0; i < tile_size; i++) {
    for (int j = 0; j < tile_size; j++) {
      progress.report_fractional_progress(curr_px_num++, num_px_to_process);
      m_seeder->update(i, j, m_seeder->seed(i, j));
    }
  }
  progress.report_finished();

  return m_seeder->result();
} 

} // namespace mvp
