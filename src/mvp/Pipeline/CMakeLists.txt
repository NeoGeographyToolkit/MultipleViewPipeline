mvp_module(NAME Pipeline
           DEPS mvpCore
                mvpImage
           HDRS 
           SRCS
           PROTOS AlgorithmSettings.proto
                  JobDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
