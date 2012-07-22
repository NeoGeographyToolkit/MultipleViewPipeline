/// \file SettingsParser.h
///
/// SettingsParser class
///
/// ...desc...
///
///

#ifndef __MVP_CORE_SETTINGSPARSER_H__
#define __MVP_CORE_SETTINGSPARSER_H__

#include <google/protobuf/message.h>

namespace mvp {
namespace core {

void parse_settings(google::protobuf::Message *proto, std::string const& filename);

}} // namespace core, mvp

#endif
