mvp_module(NAME Pipeline
           HDRS 
           SRCS
           PROTOS AlgorithmSettings.proto
                  JobDesc.proto
                  OrbitalImage.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
