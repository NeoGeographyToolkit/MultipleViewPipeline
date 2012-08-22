mvp_module(NAME OrbitalImage
           HDRS ConvexPolygon.h
                Footprint.h
                FootprintCollection.h
           SRCS ConvexPolygon.cc
                Footprint.cc
                FootprintCollection.cc
           PROTOS OrbitalImage.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
