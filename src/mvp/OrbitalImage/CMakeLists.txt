mvp_module(NAME OrbitalImage
           HDRS ConvexPolygon.h
           SRCS ConvexPolygon.cc)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
