#include <mvp/Core/Settings.h>

#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <fcntl.h>

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

}} // namespace core, mvp
