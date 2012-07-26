mvp_module(NAME Pipeline
           HDRS Session.h
           SRCS Session.cc
           PROTOS JobDesc.proto
                  SessionDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
