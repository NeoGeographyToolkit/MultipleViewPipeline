mvp_module(NAME Algorithm
           DEPS mvpCore
                mvpImage
           HDRS Dummy.h
           SRCS Dummy.cc
                Dummy/DerivedDummy.cc
           PROTOS DummySettings.proto)

#if (BUILD_TESTS)
#  add_subdirectory(tests)
#endif()
