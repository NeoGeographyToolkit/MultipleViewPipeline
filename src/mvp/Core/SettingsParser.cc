#include <mvp/Core/SettingsParser.h>

#include <google/protobuf/descriptor.h>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

#include <vw/Core/FundamentalTypes.h>
#include <vw/Core/Exception.h>

#include <iostream> // TODO: remove me

namespace mvp {
namespace core {

#define PARSE_SETTINGS_HELPER(type, setter) \
{ \
boost::optional<type> val = pt.get_optional<type>(key_name); \
if (val) { \
  item_refl->setter(sub_proto, item_field, *val); \
} else if (item_field->is_required()) { \
  vw::vw_throw(vw::NotFoundErr() << "Missing required field: " << key_name); \
} \
}

void parse_settings(google::protobuf::Message *proto, std::string const& filename) {
  using namespace google::protobuf;
  using boost::property_tree::ptree;

  ptree pt;
  read_ini(filename, pt);

  const Descriptor *section_desc = proto->GetDescriptor();
  const Reflection *section_refl = proto->GetReflection();
  int num_sections = section_desc->field_count();

  for (int i = 0; i < num_sections; i++) {
    const FieldDescriptor *section_field = section_desc->field(i);
    std::string section_name = section_field->name();
    
    if (section_field->type() != FieldDescriptor::TYPE_MESSAGE) {
      vw::vw_throw(vw::LogicErr() << "Not a message type: " << section_name);
    }

    Message *sub_proto = section_refl->MutableMessage(proto, section_field);
    const Descriptor *item_desc = sub_proto->GetDescriptor();
    const Reflection *item_refl = sub_proto->GetReflection();
    int num_fields = item_desc->field_count();

    for (int j = 0; j < num_fields; j++) {
      const FieldDescriptor *item_field = item_desc->field(j);
      std::string item_name = item_field->name();
      std::string key_name = section_name + "." + item_name;

      switch (item_field->cpp_type()) {
        case FieldDescriptor::CPPTYPE_INT32:
          PARSE_SETTINGS_HELPER(vw::int32, SetInt32);
          break;
        case FieldDescriptor::CPPTYPE_INT64:
          PARSE_SETTINGS_HELPER(vw::int64, SetInt64);
          break;
        case FieldDescriptor::CPPTYPE_UINT32:
          PARSE_SETTINGS_HELPER(vw::uint32, SetUInt32);
          break;
        case FieldDescriptor::CPPTYPE_UINT64:
          PARSE_SETTINGS_HELPER(vw::uint64, SetUInt64);
          break;
        case FieldDescriptor::CPPTYPE_DOUBLE:
          PARSE_SETTINGS_HELPER(vw::float64, SetDouble);
          break;
        case FieldDescriptor::CPPTYPE_FLOAT:
          PARSE_SETTINGS_HELPER(vw::float32, SetFloat);
          break;
        case FieldDescriptor::CPPTYPE_BOOL:
          PARSE_SETTINGS_HELPER(bool, SetBool);
          break;
        case FieldDescriptor::CPPTYPE_ENUM:
          vw::vw_throw(vw::LogicErr() << "Unsupported setting type: " << key_name);
          break;
        case FieldDescriptor::CPPTYPE_STRING:
          PARSE_SETTINGS_HELPER(std::string, SetString);
          break;
        case FieldDescriptor::CPPTYPE_MESSAGE:
          vw::vw_throw(vw::LogicErr() << "Unsupported setting type: " << key_name);
          break;
        default:
          vw::vw_throw(vw::LogicErr() << "Unsupported setting type: " << key_name);
      }
    }
  }

  // Sanity check if all required fields are set
  if (!proto->IsInitialized()) {
    vw::vw_throw(vw::LogicErr() << "Some fields still aren't initialized");
  }
}

}} // namespace core,mvp

