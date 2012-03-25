#include <iostream>

#include <mvp/MVPWorkspace.h>
#include <mvp/MVPJob.h>

#include <boost/program_options.hpp>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>
// This should probably be put where others can use it
namespace mvp {

#define PROPERTY_TO_PROTO(ptree, protobuf, type, proto_name, prop_name) \
{boost::optional<type> val = ptree.get_optional<type>(prop_name); \
if (val) (protobuf).set_##proto_name(*val);}

MVPWorkspaceRequest work_request_from_conf(std::string const& filename) {
  using boost::property_tree::ptree;
  ptree pt;

  read_ini(filename, pt);

  MVPWorkspaceRequest work_request;

  PROPERTY_TO_PROTO(pt, work_request, std::string, result_platefile, "output.result-platefile");
  PROPERTY_TO_PROTO(pt, work_request, std::string, internal_result_platefile, "output.internal-result-platefile");

  vw::cartography::Datum datum(pt.get<std::string>("platefile.datum"));
  vw::platefile::PlateGeoReference plate_georef(datum, pt.get<std::string>("platefile.map-projection"), pt.get<int>("platefile.tile-size"),
                                                vw::cartography::GeoReference::PixelAsPoint);
  *work_request.mutable_plate_georef() = plate_georef.build_desc(); 

  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), double, alt_min, "algorithm.alt-min");
  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), double, alt_max, "algorithm.alt-max");
  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), double, alt_search_range, "algorithm.alt-search-range");
  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), double, seed_window_smooth_size, "algorithm.seed-window-smooth-size");
  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), double, window_size, "algorithm.window-size");
  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), double, window_smooth_size, "algorithm.window-smooth-size");
  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), double, gauss_divisor, "algorithm.gauss-divisor");
  PROPERTY_TO_PROTO(pt, *work_request.mutable_user_settings(), int, max_iterations, "algorithm.max-iterations");

  // orbital_images

  // render_bbox
  boost::optional<std::string> render_bbox_string = pt.get_optional<std::string>("render.render-bbox");
  if (render_bbox_string) {
    typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
    boost::char_separator<char> sep(",:");

    tokenizer tokens(*render_bbox_string, sep);

    for (tokenizer::iterator tok_iter = tokens.begin();
         tok_iter != tokens.end(); tok_iter++) {
      work_request.add_render_bbox(boost::lexical_cast<int>(*tok_iter));
    }
  }

  PROPERTY_TO_PROTO(pt, work_request, int, render_level, "render.render-level");
  PROPERTY_TO_PROTO(pt, work_request, bool, use_octave, "render.use-octave");
  PROPERTY_TO_PROTO(pt, work_request, bool, draw_footprints, "render.draw-footprints");

  std::cout << work_request.DebugString() << std::endl;

  return work_request;
}

} // namespace mvp

using namespace vw;
using namespace mvp;
using namespace std;

namespace po = boost::program_options;

struct Options {
  string config_file;
  int col;
  int row;
  int level;
};

void handle_arguments(int argc, char* argv[], Options *opts) {
  po::options_description cmd_opts("Command line options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("config-file,f", po::value<string>(&opts->config_file)->required(), "Specify a pipeline configuration file")
    ("col,c", po::value<int>(&opts->col)->required(), "Column of tile to dump")
    ("row,r", po::value<int>(&opts->row)->required(), "When dumping a jobfile, row of tile to dump")
    ("level,l", po::value<int>(&opts->level)->required(), "When dumping a jobfile or printing the workspace, level to operate at")
    ;

  po::positional_options_description p;
  p.add("col", 1).add("row", 1).add("level", 1);

  po::variables_map vm; 
  store(po::command_line_parser(argc, argv).options(cmd_opts).positional(p).run(), vm);

  if (vm.count("help")) {
    vw_throw(vw::ArgumentErr() << cmd_opts);
  }

  notify(vm);
}

int main(int argc, char* argv[])
{
  Options opts;

  try {
    handle_arguments(argc, argv, &opts);
  } catch (vw::ArgumentErr const& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  MVPWorkspace work(work_request_from_conf(opts.config_file));
  MVPJob job(work.assemble_job(opts.col, opts.row, opts.level));

  string job_filename = job.save_job_file();

  vw_out() << endl << "Saved: " << job_filename << endl << endl;

  return 0;
}
