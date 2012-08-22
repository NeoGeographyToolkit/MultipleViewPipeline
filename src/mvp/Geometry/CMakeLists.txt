mvp_module(NAME Geometry
           HDRS ConvexPolygon.h
                Footprint.h
                FootprintCollection.h
           SRCS ConvexPolygon.cc
                Footprint.cc
                FootprintCollection.cc)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
