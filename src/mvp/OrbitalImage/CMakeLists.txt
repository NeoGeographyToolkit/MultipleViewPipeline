mvp_module(NAME OrbitalImage
           HDRS ConvexPolygon.h
                Footprint.h
           SRCS ConvexPolygon.cc
                Footprint.cc
           PROTOS OrbitalImage.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
