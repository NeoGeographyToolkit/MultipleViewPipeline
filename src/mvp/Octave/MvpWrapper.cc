#include <mvp/Octave/MvpWrapper.h>
#include <boost/foreach.hpp>

std::vector<MvpWrapperInstaller::OctaveFcnDesc>& MvpWrapperInstaller::installer_descs() {
  static std::vector<OctaveFcnDesc> descs;
  return descs;
}

void MvpWrapperInstaller::install_wrappers() {
  BOOST_FOREACH(OctaveFcnDesc const& item, installer_descs()) {
    install_builtin_function(item.f, item.name, item.desc);
  }
}

void MvpWrapperInstaller::add_fcn_desc(octave_builtin::fcn f, std::string name, std::string desc) {
  installer_descs().push_back(OctaveFcnDesc(f, name, desc));
}

static octave_value_list _new_mvpclass(octave_value_list const& args, int nargout) {
  return octave_value(new octave_mvpclass_ref(boost::shared_ptr<octave_mvpclass_base>(new octave_mvpclass_impl())));
}

class _Helper {
  static MvpWrapperInstallerRegistrar reg;
};

MvpWrapperInstallerRegistrar _Helper::reg(_new_mvpclass, "mvpclass", std::string());
