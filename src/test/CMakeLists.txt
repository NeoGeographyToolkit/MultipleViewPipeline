include_directories(${CMAKE_SOURCE_DIR}/thirdparty/gtest)
add_library(gtest ${CMAKE_SOURCE_DIR}/thirdparty/gtest/src/gtest-all.cc)

add_library(gtest_main test_main.cc)
add_definitions(-DTEST_SRCDIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
add_definitions(-DTEST_OBJDIR=\"${CMAKE_CURRENT_BINARY_DIR}\")
add_definitions(-DTEST_DATADIR=\"${CMAKE_SOURCE_DIR}/src/test/data\")

target_link_libraries(gtest_main gtest)

configure_file(${CMAKE_SOURCE_DIR}/src/test/loadtestenv.m.in loadtestenv.m)

