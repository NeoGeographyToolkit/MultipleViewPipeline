mvp_module(NAME Image
           HDRS ImageCrop.h
           SRCS ImageCrop.cc)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
