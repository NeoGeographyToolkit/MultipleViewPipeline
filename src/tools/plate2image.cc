#include <iostream>

#include <vw/Image/UtilityViews.h>
#include <vw/Plate/PlateView.h>
#include <vw/Plate/PlateGeoReference.h>
#include <vw/FileIO/DiskImageResourceGDAL.h>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/program_options.hpp>


// TODO: Specify transation_id

using namespace std;

using namespace vw;
using namespace vw::platefile;
using namespace vw::cartography;

namespace po = boost::program_options;

struct Options {
  po::variables_map vm;
  string plate_url;
  int level;
  string region_string;
  string region_units;
  string datum_string;
  string output_filename;
  bool dry_run;
};

void handle_arguments(int argc, char *argv[], Options *opts) {
  po::options_description cmd_opts("Options");
  cmd_opts.add_options()
    ("help,h", "Print this message")
    ("dry-run", "Print information, then exit")
    ("level,l", po::value(&opts->level)->default_value(-1), "Plate level")
    ("region", po::value(&opts->region_string), "Region as ul_x,ul_y:lr_x,lr_y")
    ("region-units", po::value<string>(&opts->region_units)->default_value("tile"), "Region units: tile,deg,px")
    ("datum,d", po::value<string>(&opts->datum_string)->required(), "Datum of platefile (D_MOON, etc)")
    ;

  po::options_description hidden_opts;
  hidden_opts.add_options()
    ("url", po::value(&opts->plate_url)->required(), "Url of platefile")
    ("output", po::value(&opts->output_filename)->required(), "Output filename")
    ;

  po::options_description all_opts;
  all_opts.add(cmd_opts).add(hidden_opts);

  po::positional_options_description p;
  p.add("url", 1).add("output", 1);

  store(po::command_line_parser(argc, argv).options(all_opts).positional(p).run(), opts->vm);

  if (opts->vm.count("help")) {
    vw_throw(ArgumentErr() << cmd_opts);
  }

  notify(opts->vm);

  opts->dry_run = opts->vm.count("dry-run");

  // Handle region
}

BBox2 parse_region_string(string const& region_string) {
  BBox2 result;
  
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",:@");

  if (region_string.empty()) {
    return BBox2();
  }

  tokenizer tokens(region_string, sep);
  tokenizer::iterator tok_iter = tokens.begin();

  if (tok_iter == tokens.end()) {
    vw_throw(ArgumentErr() << "Invalid region string: " << region_string);
  }

  for (int i = 0; i < 2; i++) {
    double x, y;

    x = boost::lexical_cast<double>(*tok_iter++);

    if (tok_iter == tokens.end()) {
      vw_throw(ArgumentErr() << "Invalid region string: " << region_string);
    }

    y = boost::lexical_cast<double>(*tok_iter++);

    result.grow(Vector2(x, y));

    if (tok_iter == tokens.end()) {
      return result;
    }
  }

  // If got here, there are still tokens left 
  vw_throw(ArgumentErr() << "Invalid region string: " << region_string);
}

template <class PixelT>
void plate2image(boost::shared_ptr<PlateFile> platefile, Options const& opts) {
  int level = opts.level;
  if (level == -1) {
    level = platefile->index_header().num_levels() - 1;
  }

  vw_out() << "Level: " << level << endl;

  // Datums and PixelIterpretation should be stored in platefiles...
  PlateGeoReference plate_geo(Datum(opts.datum_string), platefile->index_header().type(), 
                              platefile->index_header().tile_size(), GeoReference::PixelAsPoint);

  PlateView<PixelT> plateview(platefile);
  plateview.set_level(level);

  BBox2i crop_region;
  if (opts.region_string.empty()) {
    BBox2i level_bbox(0, 0, 1 << level, 1 << level);
    std::list<TileHeader> hdrs = platefile->search_by_region(level, level_bbox, TransactionRange(-1));
    BOOST_FOREACH(TileHeader const& h, hdrs) {
      crop_region.grow(plate_geo.tile_pixel_bbox(h.col(), h.row(), h.level()));
    }
  } else {
    BBox2 parsed_region = parse_region_string(opts.region_string);

    if (opts.region_units == "tile") {
      BBox2i int_region = vw::grow_bbox_to_int(parsed_region);
      crop_region.grow(plate_geo.tile_pixel_bbox(int_region.min().x(), int_region.min().y(), level));
      crop_region.grow(plate_geo.tile_pixel_bbox(int_region.max().x(), int_region.max().y(), level));
    } else if (opts.region_units == "deg") {
      GeoReference georef(plate_geo.level_georef(level));
      BBox2 deg_region;
      deg_region.grow(georef.lonlat_to_pixel(parsed_region.min()));
      deg_region.grow(georef.lonlat_to_pixel(parsed_region.max()));
      crop_region = vw::grow_bbox_to_int(deg_region); 
    } else if (opts.region_units == "px") {
      crop_region = vw::grow_bbox_to_int(parsed_region);
    } else {
      vw_throw(ArgumentErr() << "Invalid region units: " << opts.region_units);
    }
  }

  vw_out() << "Region (in px): " << crop_region << endl;
  vw_out() << "Region width: " << crop_region.width() << " height: " << crop_region.height() << endl;

  if (opts.dry_run) {
    vw_out() << "Write to: " << opts.output_filename << endl;
    vw_out() << "Dry run requested, exiting..." << endl;
    exit(0);
  }

  ImageViewRef<PixelT> out_image = crop(plateview, crop_region);
  GeoReference out_georef = crop(plate_geo.level_georef(level), crop_region);

  DiskImageResourceGDAL rsrc(opts.output_filename, out_image.format(), Vector2i(plate_geo.tile_size(), plate_geo.tile_size()));
  write_georeference(rsrc, out_georef);
  write_image(rsrc, out_image, TerminalProgressCallback("vw", opts.output_filename + ": "));
}

int main(int argc, char *argv[]) {
  Options opts;

  try {
    handle_arguments(argc, argv, &opts);

    boost::shared_ptr<PlateFile> platefile(new PlateFile(opts.plate_url));

     PixelFormatEnum pixel_format = platefile->pixel_format();
     ChannelTypeEnum channel_type = platefile->channel_type();

     switch (pixel_format) {
       case VW_PIXEL_GRAYA:
         switch(channel_type) {
           case VW_CHANNEL_UINT8:
             plate2image<PixelGrayA<uint8> >(platefile, opts);
             break;
           case VW_CHANNEL_INT16:
             plate2image<PixelGrayA<int16> >(platefile, opts);
             break;
           case VW_CHANNEL_FLOAT32:
             plate2image<PixelGrayA<float32> >(platefile, opts);
             break;
           default:
             vw_throw(ArgumentErr() << "Platefile contains a channel type not supported by plate2image.");
         }
         break;
       case VW_PIXEL_RGB:
       case VW_PIXEL_RGBA:
       default:
         switch(channel_type) {
           case VW_CHANNEL_UINT8:
             plate2image<PixelRGBA<uint8> >(platefile, opts);
             break;
           default:
             vw_throw(ArgumentErr() << "Platefile contains a channel type not supported by plate2image.");
         }
         break;
     }
  } catch (ArgumentErr const& e) {
    vw_out() << e.what() << endl;
    return 1;
  }

  return 0;
}
