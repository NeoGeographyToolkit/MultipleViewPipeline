mvp_module(NAME Pipeline
           DEPS mvpCore
           HDRS 
           SRCS
           PROTOS AlgorithmSettings.proto
                  JobDesc.proto
                  OrbitalImage.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
