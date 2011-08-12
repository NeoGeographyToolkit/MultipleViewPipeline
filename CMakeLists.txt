cmake_minimum_required (VERSION 2.6)
project (AMVP)

include(CMakeDependentOption)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

add_subdirectory(src)

find_package(Octave)

if (OCTAVE_FOUND)
  option(BUILD_OCTAVEVW "Build the octave->vw bridge" ON)
else()
  option(BUILD_OCTAVEVW "Build the octave->vw bridge" OFF)
endif()

option(BUILD_TOOLS "Build the tools module" ON)

option(BUILD_ORBITFOOT "Build the orbitfoot utility" ON)
