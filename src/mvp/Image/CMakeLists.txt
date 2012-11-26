mvp_module(NAME Image
           DEPS mvpCore
           HDRS OrbitalImage.h
                OrbitalImageCollection.h
           SRCS OrbitalImage.cc
                OrbitalImageCollection.cc
           PROTOS OrbitalImageDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
