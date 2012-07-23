mvp_module(NAME Pipeline
           HDRS
           SRCS
           PROTOS JobDesc.proto
                  SessionDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
