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

MVPTileResult MVPTileProcessorSquare::operator()(vw::ProgressCallback const& progress) const {
  if (!m_seeder->init()) {
    progress.report_finished();
    return m_seeder->result();    
  }

  int tile_size = m_seeder->result().tile_size;
  int tile_mid = tile_size / 2;

  int curr_px_num = 0;
  int num_px_to_process = tile_size * tile_size;
  for (int i = tile_mid; i < tile_size; i++) {
    for (int j = i; j >= tile_size - i - 1; j--) {
      progress.report_fractional_progress(curr_px_num++, num_px_to_process);
      m_seeder->update(i, j, m_seeder->seed(i, j));

      if (i != j) {
        progress.report_fractional_progress(curr_px_num++, num_px_to_process);
        m_seeder->update(j, i, m_seeder->seed(j, i));
      }

      int rev_i = tile_size - i - 1;
      int rev_j = tile_size - j - 1;

      if (rev_i != j) {
        progress.report_fractional_progress(curr_px_num++, num_px_to_process);
        m_seeder->update(rev_i, rev_j, m_seeder->seed(rev_i, rev_j));

        if (rev_i != rev_j) {
          progress.report_fractional_progress(curr_px_num++, num_px_to_process);
          m_seeder->update(rev_j, rev_i, m_seeder->seed(rev_j, rev_i));
        }
      }
    }
  }
  progress.report_finished();

  return m_seeder->result();
}

} // namespace mvp
