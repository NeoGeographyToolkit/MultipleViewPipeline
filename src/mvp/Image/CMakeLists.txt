mvp_module(NAME Image
           DEPS mvpCore
                mvpPipeline
           HDRS ImageCrop.h
                ImageCropCollection.h
           SRCS ImageCrop.cc
                ImageCropCollection.cc)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
