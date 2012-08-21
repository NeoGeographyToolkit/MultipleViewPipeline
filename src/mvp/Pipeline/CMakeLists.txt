mvp_module(NAME Pipeline
           HDRS ConvexPolygon.h
           SRCS ConvexPolygon.cc
           PROTOS JobDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
