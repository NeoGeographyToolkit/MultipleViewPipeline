mvp_module(NAME Pipeline
           DEPS mvpCore
                mvpImage
           HDRS Session.h
                Job.h
           SRCS Session.cc
                Job.cc
           PROTOS AlgorithmSettings.proto
                  JobDesc.proto
                  SessionDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
