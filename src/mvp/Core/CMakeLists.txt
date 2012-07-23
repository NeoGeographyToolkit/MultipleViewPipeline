mvp_module(NAME Core
           HDRS 
           SRCS
           PROTOS GlobalSettings.proto
                  AlgorithmSettings.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
