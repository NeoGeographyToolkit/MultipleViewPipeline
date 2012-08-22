mvp_module(NAME Core
           HDRS Settings.h
           SRCS Settings.cc
           PROTOS GlobalSettings.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
