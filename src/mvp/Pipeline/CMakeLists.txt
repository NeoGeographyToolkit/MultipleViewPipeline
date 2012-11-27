mvp_module(NAME Pipeline
           DEPS mvpCore
                mvpImage
           HDRS Session.h
           SRCS Session.cc
           PROTOS AlgorithmSettings.proto
                  JobDesc.proto
                  SessionDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
