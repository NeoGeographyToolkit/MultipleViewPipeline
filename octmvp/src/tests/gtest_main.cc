#include <octave/oct.h>
#include <octave/octave.h>
#include <octave/toplev.h>

#include <gtest/gtest.h>

int main(int argc, char **argv) {
  const char * argvv [] = {"", "--silent"};
  ::octave_main (2, (char **) argvv, true);

  testing::InitGoogleTest(&argc, argv);
  int status = RUN_ALL_TESTS();

  do_octave_atexit();

  return status;
}
