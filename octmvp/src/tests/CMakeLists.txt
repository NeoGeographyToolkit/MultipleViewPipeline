include_directories(${MVP_DIR}/thirdparty/gtest/include)
include_directories(${MVP_DIR}/thirdparty/gtest)
add_library(gtest_main gtest_main.cc
                       ${MVP_DIR}/thirdparty/gtest/src/gtest-all.cc)
target_link_libraries(gtest_main pthread)

include_directories(..)

add_executable(TestMathSupport TestMathSupport.cxx)
target_link_libraries(TestMathSupport gtest_main ${OCTAVE_LIBRARIES})

add_test(TestMathSupport TestMathSupport)
