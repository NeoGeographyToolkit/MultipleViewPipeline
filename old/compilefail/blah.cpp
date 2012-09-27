#include <octave/oct.h>                                                                                                                                              
#include <octave/parse.h>                                                                                                                                            
#include <octave/octave.h>                                                                                                                                           
#include <octave/toplev.h>

#include <iostream>

void start() {
  const char * argvv [] = {"", "--silent"};
  ::octave_main (2, (char **) argvv, true);
}

void end() {
  std::cout << "eh" << std::endl;
  do_octave_atexit();
}
