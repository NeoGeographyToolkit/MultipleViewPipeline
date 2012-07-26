#include <mvp/Core/Settings.h>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fcntl.h>

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

namespace mvp {
namespace core {

GlobalSettings& mvp_settings(std::string const& loadfrom) {
  static GlobalSettings set;
  static bool first = true;

  if (first) {
    first = false;
  }

  if (!loadfrom.empty()) {
    load_settings(&set, loadfrom);
  }

  return set;
}

void load_settings(google::protobuf::Message *message, std::string const& filename) {
  namespace gp = google::protobuf;

  int fd = open(filename.c_str(), O_RDONLY);
  if (fd == -1) {
    // TODO: throw here!
  }
  gp::io::FileInputStream stream(fd);
  gp::TextFormat::Merge(&stream, message);
}


vw::BBox2 parse_bbox_string(std::string const& bbox_string) {
  using namespace vw;

  BBox2 result;
  
  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
  boost::char_separator<char> sep(",:@");

  if (bbox_string.empty()) {
    return BBox2();
  }

  tokenizer tokens(bbox_string, sep);
  tokenizer::iterator tok_iter = tokens.begin();

  if (tok_iter == tokens.end()) {
    vw_throw(ArgumentErr() << "Invalid bbox string: " << bbox_string);
  }

  for (int i = 0; i < 2; i++) {
    double x, y;

    x = boost::lexical_cast<double>(*tok_iter++);

    if (tok_iter == tokens.end()) {
      vw_throw(ArgumentErr() << "Invalid bbox string: " << bbox_string);
    }

    y = boost::lexical_cast<double>(*tok_iter++);

    result.grow(Vector2(x, y));

    if (tok_iter == tokens.end()) {
      return result;
    }
  }

  // If got here, there are still tokens left 
  vw_throw(ArgumentErr() << "Invalid bbox string: " << bbox_string);
}

}} // namespace core, mvp
