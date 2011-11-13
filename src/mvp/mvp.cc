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

template <class BBoxT, class RealT, size_t DimN>
void print_bbox_helper(math::BBoxBase<BBoxT, RealT, DimN> const& bbox) {
  cout << "BBox = " << bbox << endl;
  cout << "width, height = " << bbox.width() << ", " << bbox.height() << endl;
}


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
    ("print-workspace,p", "Print information about the workspace and exit")
    ("dump-job", "Dump a jobfile")
    ("col,c", po::value<int>(), "When dumping a jobfile, column of tile to dump")
    ("row,r", po::value<int>(), "When dumping a jobfile, row of tile to dump")
    ("level,l", po::value<int>(), "When dumping a jobfile or printing the workspace, level to operate at")
    ;

  po::options_description render_opts("Render Options");
  render_opts.add_options()
    ("col-start", po::value<int>(), "Col to start rendering at")
    ("col-end", po::value<int>(), "One past last col to render")
    ("row-start", po::value<int>(), "Row to start rendering at")
    ("row-end", po::value<int>(), "One past last row to render")
    ("render-level", po::value<int>(), "Level to render at")
    ;

  po::options_description mvp_opts;
  mvp_opts.add(MVPWorkspace::program_options()).add(render_opts);

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

  if (!vm.count("silent")) {
    cout << boolalpha << endl;
    cout << "-------------------------------------" << endl;
    cout << "Welcome to the Multiple View Pipeline" << endl;
    cout << "-------------------------------------" << endl;
    cout << endl;
    cout << "Number of images loaded = " << work.num_images() << endl;
    cout << " Equal resolution level = " << work.equal_resolution_level() << endl;
    cout << "    Equal density level = " << work.equal_density_level() << endl;
    cout << endl;
    cout << "# Workspace lonlat BBox #" << endl;
    print_bbox_helper(work.lonlat_work_area());
    cout << endl;
    cout << "# Workspace tile BBox (@ equal density level) #" << endl;
    print_bbox_helper(work.tile_work_area(work.equal_density_level()));
    if (vm.count("level")) {
      int print_level = vm["level"].as<int>();
      cout << endl;
      cout << "# Workspace tile BBox (@ level " << print_level << ") #" << endl;
      print_bbox_helper(work.tile_work_area(print_level));
    }
    cout << endl;
  } 

  if (vm.count("print-workspace")) {
    return 0;
  }

  int render_level = work.equal_density_level();
  if (vm.count("render-level")) {
    render_level = vm["render-level"].as<int>();
  }

  BBox2i tile_bbox(work.tile_work_area(render_level));

  if (vm.count("col-start")) {
    VW_ASSERT(vm.count("col-end"), ArgumentErr() << "col-start specified, but col-end not");
    tile_bbox.min()[0] = vm["col-start"].as<int>();
    tile_bbox.max()[0] = vm["col-end"].as<int>();
  }

  if (vm.count("row-start")) {
    VW_ASSERT(vm.count("row-end"), ArgumentErr() << "row-start specified, but col-end not");
    tile_bbox.min()[1] = vm["row-start"].as<int>();
    tile_bbox.max()[1] = vm["row-end"].as<int>();
  }

  if (!vm.count("silent")) {
    cout << "-------------------------------------" << endl;
    cout << "        Rendering Information" << endl;
    cout << "-------------------------------------" << endl;
    cout << endl;
    cout << "Render level = " << render_level << endl;
    cout << "  Use octave = " << vm["use-octave"].as<bool>() << endl;
    cout << endl;
    cout << "# Render tile BBox #" << endl;
    print_bbox_helper(tile_bbox);
    cout << endl;
    cout << "-------------------------------------" << endl;
    cout << "              Status" << endl;
    cout << "-------------------------------------" << endl;
    cout << endl;
  }

  boost::shared_ptr<PlateFile> pf(new PlateFile(work.result_platefile(),
                                                work.plate_georef().map_proj(),
                                                "MVP Result Plate",
                                                work.plate_georef().tile_size(),
                                                "tif", VW_PIXEL_GRAYA, VW_CHANNEL_FLOAT32));

  Transaction tid = pf->transaction_request("Post Heights", -1);

  int curr_tile = 0;
  int num_tiles = tile_bbox.width() * tile_bbox.height();
  float32 plate_min_val = numeric_limits<float32>::max(), plate_max_val = numeric_limits<float32>::min();
  for (int col = tile_bbox.min().x(); col < tile_bbox.max().x(); col++) {
    for (int row = tile_bbox.min().y(); row < tile_bbox.max().y(); row++) {
      boost::shared_ptr<ProgressCallback> progress;

      if (!vm.count("silent")) {
        ostringstream status;
        status << "Tile: " << ++curr_tile << "/" << num_tiles << " Location: [" << col << ", " << row << "] @" << render_level << " ";
        progress.reset(new TerminalProgressCallback("mvp", status.str()));
      } else {
        progress.reset(new ProgressCallback);
      }

      MVPTileResult result = mvpjob_process_tile(work.assemble_job(col, row, render_level), *progress);
      
      ImageView<PixelGrayA<float32> > rendered_tile = mask_to_alpha(pixel_cast<PixelMask<PixelGray<float32> > >(result.post_height));

      float32 tile_min_val, tile_max_val;
      try {
        min_max_channel_values(result.post_height, tile_min_val, tile_max_val);
      } catch (ArgumentErr& e) {
        tile_min_val = numeric_limits<float32>::max();
        tile_max_val = numeric_limits<float32>::min(); 
      }

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
        ImageView<PixelGrayA<float32> > rendered_tile(constant_view(PixelGrayA<float32>(), 
                                                                    work.plate_georef().tile_size(), work.plate_georef().tile_size()));
        pf->write_request();
        pf->write_update(rendered_tile, col, row, level, tid);
        pf->sync();
        pf->write_complete();
      }
    }
  }

  if (!vm.count("silent")) {
    cout << "Plate (min, max): (" << plate_min_val << ", " << plate_max_val << ")" << endl;
    cout << endl;
  }

  return 0;
}
