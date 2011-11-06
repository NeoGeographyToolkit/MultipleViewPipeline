#include <iostream>

#include <vw/Image/ImageMath.h>
#include <vw/Image/UtilityViews.h>
#include <vw/Plate/PlateFile.h>
#include <vw/Image/MaskViews.h>

#include <mvp/Config.h>
#include <mvp/MVPWorkspace.h>
#include <mvp/MVPJob.h>

#include <boost/program_options.hpp>

using namespace vw;
using namespace vw::cartography;
using namespace vw::platefile;
using namespace std;
using namespace mvp;

namespace po = boost::program_options;

int main(int argc, char* argv[])
{
  #if MVP_ENABLE_OCTAVE_SUPPORT
  MVPJobOctave::start_interpreter();
  #endif

  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("silent", "Run without outputting status")
    ("config-file,f", po::value<string>()->default_value("mvp.conf"), "Specify a pipeline configuration file")
    ;

  po::options_description mvp_opts;
  mvp_opts.add(MVPWorkspace::program_options());

  po::options_description all_opts;
  all_opts.add(cmd_opts).add(mvp_opts);

  po::variables_map vm;
  store(po::command_line_parser(argc, argv).options(all_opts).run(), vm);

  if (vm.count("help")) {
    cout << all_opts << endl;
    return 0;
  }

  ifstream ifs(vm["config-file"].as<string>().c_str());
  if (ifs) {
    store(parse_config_file(ifs, mvp_opts), vm);
  }

  notify(vm);

  MVPWorkspace work(MVPWorkspace::construct_from_program_options(vm)); 

  int render_level = work.equal_resolution_level();

  BBox2i tile_bbox(work.tile_work_area(render_level));

  boost::shared_ptr<PlateFile> pf(new PlateFile(work.result_platefile(),
                                                work.plate_georef().map_proj(),
                                                "desc",
                                                work.plate_georef().tile_size(),
                                                "tif", VW_PIXEL_GRAYA, VW_CHANNEL_FLOAT32));

  Transaction tid = pf->transaction_request("tdesc", -1);

  int curr_tile = 0;
  int num_tiles = tile_bbox.width() * tile_bbox.height();
  float32 plate_min_val = numeric_limits<float32>::max(), plate_max_val = numeric_limits<float32>::min();
  for (int col = tile_bbox.min().x(); col < tile_bbox.max().x(); col++) {
    for (int row = tile_bbox.min().y(); row < tile_bbox.max().y(); row++) {
      ostringstream status;
      status << "Tile: " << ++curr_tile << "/" << num_tiles << " Location: [" << col << ", " << row << "] @" << render_level << " ";

      MVPTileResult result = mvpjob_process_tile(work.assemble_job(col, row, render_level), TerminalProgressCallback("mvp", status.str()));
      
      ImageView<PixelGrayA<float32> > rendered_tile = mask_to_alpha(pixel_cast<PixelMask<PixelGray<float32> > >(result.post_height));

      float32 tile_min_val, tile_max_val;
      min_max_channel_values(result.post_height, tile_min_val, tile_max_val);
      plate_min_val = min(plate_min_val, tile_min_val);
      plate_max_val = max(plate_max_val, tile_max_val);

      pf->write_request();
      pf->write_update(rendered_tile, col, row, render_level, tid);
      pf->sync();
      pf->write_complete();
    }
  }

  // This way that tile is easy to find...
  for (int level = 2; level < render_level; level++) {
    int divisor = render_level - level;
    for (int col = tile_bbox.min().x() >> divisor; col <= tile_bbox.max().x() >> divisor; col++) {
      for (int row = tile_bbox.min().y() >> divisor; row <= tile_bbox.max().y() >> divisor; row++) {
        ImageView<PixelGrayA<float32> > rendered_tile(constant_view(PixelGrayA<float32>(plate_max_val, 1), 
                                                                    work.plate_georef().tile_size(), work.plate_georef().tile_size()));
        pf->write_request();
        pf->write_update(rendered_tile, col, row, level, tid);
        pf->sync();
        pf->write_complete();
      }
    }
  }

  cout << "Plate (min, max): (" << plate_min_val << ", " << plate_max_val << ")" << endl;

  return 0;
}
