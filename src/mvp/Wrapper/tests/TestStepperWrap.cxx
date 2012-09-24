#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <octave/config.h>
#include <mvp/Wrapper/Stepper.h>

using namespace mvp::wrapper;
using namespace mvp::algorithm;
using namespace vw::test;
using namespace std;

TEST(Stepper, zing) {
  boost::shared_ptr<Stepper> step(new OctaveStepper("StupidStepper"));
  
  cout << step->zap(100) << endl;
}
