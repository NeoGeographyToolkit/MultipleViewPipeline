#include <gtest/gtest.h>
#include <test/Helpers.h>

#include <mvp/Octave/OctaveCast.h>

using namespace mvp::octave;
using namespace vw::test;
using namespace std;

TEST(Conversions, scalar_to_octave) {
  EXPECT_TRUE((octave_cast<octave_value>(5) == octave_value(5)).bool_value());
}

TEST(Conversions, octave_to_scalar) {
  EXPECT_EQ(octave_cast<int>(octave_value(5)), 5);
}

TEST(Conversions, octave_to_scalar_throws) {
  EXPECT_THROW(octave_cast<int>(octave_map()), BadCastErr);
}

TEST(Conversions, vector_to_octave) {
  vw::Vector3 vw_vect(10, 20, 30);
  ColumnVector oct_vect(octave_cast<ColumnVector>(vw_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, octave_to_vector) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector3 vw_vect(octave_cast<vw::Vector3>(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, octave_to_vector0) {
  ColumnVector oct_vect(3);
  oct_vect(0) = 10; oct_vect(1) = 20; oct_vect(2) = 30;

  vw::Vector<double, 0> vw_vect(octave_cast<vw::Vector<double, 0> >(oct_vect));

  for (unsigned i = 0; i < vw_vect.size(); i++) {
    EXPECT_EQ(vw_vect[i], oct_vect(i));
  }
}

TEST(Conversions, octave_to_vector_throws) {
  EXPECT_THROW(octave_cast<vw::Vector2>(ColumnVector(3)), BadCastErr);
  EXPECT_THROW(octave_cast<vw::Vector2>(octave_map()), BadCastErr);
}
