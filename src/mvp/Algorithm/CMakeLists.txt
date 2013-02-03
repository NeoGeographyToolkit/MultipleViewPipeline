mvp_module(NAME Algorithm
           DEPS mvpCore
           HDRS Dummy.h
           SRCS Dummy.cc)

#if (BUILD_TESTS)
#  add_subdirectory(tests)
#endif()
