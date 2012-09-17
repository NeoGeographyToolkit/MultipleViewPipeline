mvp_module(NAME Octave
           HDRS Main.h
           SRCS Main.cc)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
