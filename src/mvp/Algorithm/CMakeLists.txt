mvp_module(NAME Algorithm
           DEPS mvpCore
           HDRS Dummy.h
           SRCS Dummy/DerivedDummy.cc)

#if (BUILD_TESTS)
#  add_subdirectory(tests)
#endif()
