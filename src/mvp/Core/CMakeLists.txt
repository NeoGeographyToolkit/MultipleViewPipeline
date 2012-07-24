mvp_module(NAME Core
           HDRS 
           SRCS
           PROTOS GlobalSettings.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
