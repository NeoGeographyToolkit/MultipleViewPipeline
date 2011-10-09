cmake_minimum_required (VERSION 2.6)
project (AMVP)

include(CMakeDependentOption)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(GTEST_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/gtest/build)

# This is to avoid the bug http://public.kitware.com/Bug/view.php?id=12442
# Otherwise, if /packages version does not have MT but the system distribution
# does, it will incorrectly use the system version. It would be a good idea
# to periodically check which boost libs the programs are linking to by running
# ldd on them...
set(Boost_USE_MULTITHREADED OFF)

find_package(Octave)

if (OCTAVE_FOUND)
  option(BUILD_OCTAVEVW "Build the octave->vw bridge" ON)
else()
  option(BUILD_OCTAVEVW "Build the octave->vw bridge" OFF)
endif()

option(BUILD_TOOLS "Build the tools module" ON)

option(BUILD_ORBITFOOT "Build the orbitfoot utility" ON)

option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
  enable_testing()
endif()


include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)
add_subdirectory(src)
