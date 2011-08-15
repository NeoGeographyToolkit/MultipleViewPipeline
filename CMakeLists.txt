cmake_minimum_required (VERSION 2.6)
project (AMVP)

include(CMakeDependentOption)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(GTEST_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/gtest)

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

add_subdirectory(src)
