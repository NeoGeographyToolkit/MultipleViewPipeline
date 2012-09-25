mvp_module(NAME Wrapper
           DEPS mvpCore
                mvpOctave
           HDRS OctaveWrapper.h
                MvpWrapper.h
           SRCS OctaveWrapper.cc)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
