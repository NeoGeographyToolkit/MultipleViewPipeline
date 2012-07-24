/// \file Settings.h
///
/// Defines mvp_settings()
///
/// TODO: Write something here
///

#ifndef __MVP_CORE_SETTINGS__
#define __MVP_CORE_SETTINGS__

#include <mvp/Core/GlobalSettings.pb.h>

namespace mvp {
namespace core {

GlobalSettings& mvp_settings(std::string const& loadfrom = "");

void load_settings(google::protobuf::Message *message, std::string const& filename);

}} // namespace core, mvp

namespace mvp {
  using core::mvp_settings;
} // namespace core

#endif
