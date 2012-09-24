#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <boost/scoped_ptr.hpp>

#include <octave/config.h>

#include <mvp/Wrapper/Dummy.h>

using namespace mvp::algorithm;
using namespace mvp::wrapper;
using namespace vw::test;
using namespace std;

TEST(OctaveWrapper, construct) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy"));
  EXPECT_EQ(dummy->x(), 0);
  EXPECT_EQ(dummy->y(), 0);
}

TEST(OctaveWrapper, construct1) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy", 1));
  EXPECT_EQ(dummy->x(), 1);
  EXPECT_EQ(dummy->y(), 0);
}

TEST(OctaveWrapper, construct2) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy", 1, 2));
  EXPECT_EQ(dummy->x(), 1);
  EXPECT_EQ(dummy->y(), 2);
}

TEST(OctaveWrapper, void0) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy"));
  dummy->void0();
}

TEST(OctaveWrapper, void1) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy"));
  dummy->void1(1);
}

TEST(OctaveWrapper, void2) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy"));
  dummy->void2(1, 2);
}

TEST(OctaveWrapper, function0) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy"));
  EXPECT_EQ(dummy->function0(), 0);
}

TEST(OctaveWrapper, function1) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy"));
  EXPECT_EQ(dummy->function1(1), 1);
}

TEST(OctaveWrapper, function2) {
  boost::scoped_ptr<Dummy> dummy(new OctaveDummy("DerivedDummy"));
  EXPECT_EQ(dummy->function2(1, 2), 3);
}
