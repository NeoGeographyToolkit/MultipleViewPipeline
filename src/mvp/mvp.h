#ifndef __MVP_MVP_H__
#define __MVP_MVP_H__

#include <iostream>

#include <vw/Image/ImageMath.h>
#include <vw/Image/UtilityViews.h>
#include <vw/Plate/PlateFile.h>
#include <vw/Image/MaskViews.h>
#include <vw/Octave/Main.h>

#include <mvp/Config.h>
#include <mvp/MVPWorkspace.h>
#include <mvp/MVPJob.h>

#include <boost/program_options.hpp>

using namespace vw;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace vw::octave;
using namespace std;
using namespace mvp;

namespace po = boost::program_options;

void add_nongearman_task(MVPJobRequest const& job_request, int curr_tile, int num_tiles, bool silent = false) {
  boost::shared_ptr<ProgressCallback> progress;
  if (!silent) {
    ostringstream status;
    status << "Tile: " << curr_tile << "/" << num_tiles << " Location: [" << job_request.col() << ", " << job_request.row() << "] @" << job_request.level() << " ";
    progress.reset(new TerminalProgressCallback("mvp", status.str()));
  } else {
    progress.reset(new ProgressCallback);
  }
  mvpjob_process_and_write_tile(job_request, *progress);
}

void plate_tunnel(MVPWorkspace const& work, BBox2i const& tile_bbox, int render_level) {
  boost::scoped_ptr<PlateFile> pf(new PlateFile(work.result_platefile(),
                                                work.plate_georef().map_proj(),
                                                "MVP Result Plate",
                                                work.plate_georef().tile_size(),
                                                "tif", VW_PIXEL_GRAYA, VW_CHANNEL_FLOAT32));

  pf->transaction_begin("Tunnel", 3);
  pf->write_request();

  // This way that tile is easy to find...
  for (int level = 2; level < render_level; level++) {
    int divisor = render_level - level;
    for (int col = tile_bbox.min().x() >> divisor; col <= tile_bbox.max().x() >> divisor; col++) {
      for (int row = tile_bbox.min().y() >> divisor; row <= tile_bbox.max().y() >> divisor; row++) {
        ImageView<PixelGrayA<float32> > rendered_tile(constant_view(PixelGrayA<float32>(), 
                                                                    work.plate_georef().tile_size(), work.plate_georef().tile_size()));
        pf->write_update(rendered_tile, col, row, level);
      }
    }
  }

  pf->sync();
  pf->write_complete();
  pf->transaction_end(true);
}

template <class BBoxT, class RealT, size_t DimN>
void print_bbox_helper(math::BBoxBase<BBoxT, RealT, DimN> const& bbox) {
  cout << "BBox = " << bbox << endl;
  cout << "width, height = " << bbox.width() << ", " << bbox.height() << endl;
}


#endif
