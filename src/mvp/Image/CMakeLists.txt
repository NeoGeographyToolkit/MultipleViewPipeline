mvp_module(NAME Image
           DEPS mvpCore
           HDRS OrbitalImage.h
                OrbitalImageCollection.h
                OrbitalImageCatalog.h
           SRCS OrbitalImage.cc
                OrbitalImageCollection.cc
                OrbitalImageCatalog.cc
           PROTOS OrbitalImageDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
