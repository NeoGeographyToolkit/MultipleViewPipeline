mvp_module(NAME Frontend
           HDRS Session.h
           SRCS Session.cc
           PROTOS SessionDesc.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
