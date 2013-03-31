mvp_module(NAME Algorithm
           DEPS mvpCore
                mvpImage
           HDRS Dummy.h
           SRCS Dummy.cc
                Dummy/DerivedDummy.cc
                AlgorithmVar.cc
                PixelResult.cc
                Seed.cc
                TileResult.cc
           PROTOS DummySettings.proto)

#if (BUILD_TESTS)
#  add_subdirectory(tests)
#endif()
