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


#if MVP_ENABLE_GEARMAN_SUPPORT
#include <libgearman/gearman.h>

void print_statuses(std::list<gearman_task_st *> const& tasks, std::list<gearman_task_st *> const& statuses) {
  assert(tasks.size() == statuses.size());
  static int last_print_size = 0;
  std::stringstream stream;

  std::list<gearman_task_st *>::const_iterator task_iterator = tasks.begin();
  std::list<gearman_task_st *>::const_iterator status_iterator = statuses.begin();
  while (task_iterator != tasks.end()) {
    gearman_task_st *t = *(task_iterator++);
    gearman_task_st *s = *(status_iterator++);

    if (gearman_task_denominator(s)) {
      double percent = 100 * gearman_task_numerator(s) / gearman_task_denominator(s);
      stream << "Task #" << gearman_task_unique(t) << ": [" << percent << "%] ";
    }
  }
  std::cout << std::setw(last_print_size) << std::left << stream.str() << "\r" << std::flush;
  last_print_size = stream.str().size();
}

enum TaskWaitType {
  UNTIL_EVERYTHING_IS_RUNNING,
  UNTIL_EVERYTHING_IS_DONE
};

std::list<gearman_task_st *> wait_on_gearman_tasks(gearman_client_st *client, std::list<gearman_task_st *> const& tasks, TaskWaitType wait_type) {
  bool keep_looping;
  std::list<gearman_task_st *> statuses;
  gearman_return_t ret;

  do {
    BOOST_FOREACH(gearman_task_st *t, tasks) {
      statuses.push_back(gearman_client_add_task_status(client, 0, 0, gearman_task_job_handle(t), &ret));
    }
    gearman_client_run_tasks(client);
    
    print_statuses(tasks, statuses);

    keep_looping = false;

    BOOST_FOREACH(gearman_task_st *s, statuses) {
      switch (wait_type) {
        case UNTIL_EVERYTHING_IS_RUNNING:
          if (gearman_task_is_known(s) && !gearman_task_is_running(s)) {
            keep_looping = true;
          }
          break;
        case UNTIL_EVERYTHING_IS_DONE:
          if (gearman_task_is_known(s)) {
            keep_looping = true;
          }
          break;
        default:
          assert(0);
      }
    }

    if (keep_looping) {
      BOOST_FOREACH(gearman_task_st *s, statuses) {
        gearman_task_free(s);
      }
      statuses.clear();
      sleep(1);
    }
  } while (keep_looping);

  return statuses;
}

void add_gearman_task(gearman_client_st *client, std::list<gearman_task_st *> *tasks, 
                      MVPJobRequest const& job_request, int curr_tile, int num_tiles, bool silent = false) 
{
  gearman_return_t ret;

  // TODO: Need a better unique, otherwise will clash if multiple mvp clients are running...
  stringstream ss;
  ss << curr_tile;

  string message;
  job_request.SerializeToString(&message);

  tasks->push_back(gearman_client_add_task_background(client, 0, 0, "mvpalgorithm", ss.str().c_str(), message.c_str(), message.size(), &ret));
  gearman_client_run_tasks(client);

  std::list<gearman_task_st *> statuses = wait_on_gearman_tasks(client, *tasks, UNTIL_EVERYTHING_IS_RUNNING);

  assert(statuses.size() == tasks->size());

  std::list<gearman_task_st *>::iterator task_iterator = tasks->begin();
  std::list<gearman_task_st *>::const_iterator status_iterator = statuses.begin();

  while (task_iterator != tasks->end()) {
    if (!gearman_task_is_known(*status_iterator)) {
      gearman_task_free(*task_iterator);
      task_iterator = tasks->erase(task_iterator);
    } else {
      task_iterator++;
    }
    gearman_task_free(*status_iterator);
    status_iterator++;
  }
}
#endif

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
