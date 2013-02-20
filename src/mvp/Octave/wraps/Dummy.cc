#include <mvp/Octave/MvpWrapper.h>
#include <mvp/Algorithm/Dummy.h>

octave_value_list _new_Dummy(octave_value_list const& args, int nargout) {
  if (args.length() == 0) {
    error("No dummy type specified");
    return octave_value();
  }

  std::string name = args(0).string_value();

  return mvp::octave::octave_wrap(mvp::algorithm::Dummy(name, 5, 6));
}

class k_Helper {
  static MvpWrapperInstallerRegistrar reg;
};

MvpWrapperInstallerRegistrar k_Helper::reg(_new_Dummy, "Dummy", std::string());

BEGIN_MVP_WRAPPER(mvp::algorithm::Dummy)
  MVP_WRAP(void0)
  MVP_WRAP(void1)
  MVP_WRAP(void2)
  MVP_WRAP(function0)
  MVP_WRAP(function1)
  MVP_WRAP(function2)
  MVP_WRAP(do_vector)
END_MVP_WRAPPER()
