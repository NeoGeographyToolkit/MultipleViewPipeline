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

template <>
class MvpWrapperInstallerRegistrar<void> {
  static MvpWrapperInstallerRegistrar<void> reg;
  static octave_value_list construct_fcn(octave_value_list const& args, int nargout) {
    return octave_value(new octave_mvpclass_ref(boost::shared_ptr<octave_mvpclass_base>(new octave_mvpclass_impl())));
  }
  public:
    MvpWrapperInstallerRegistrar(std::string name, std::string desc) {
      MvpWrapperInstaller::add_fcn_desc(construct_fcn, name, desc);
    }
};

MvpWrapperInstallerRegistrar<void> MvpWrapperInstallerRegistrar<void>::reg("mvpclass", std::string());
