cmake_minimum_required (VERSION 2.6)
project (MVP)

include(CMakeDependentOption)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(GTEST_ROOT ${CMAKE_SOURCE_DIR}/thirdparty/gtest/build)

# This is to avoid the bug http://public.kitware.com/Bug/view.php?id=12442
# Otherwise, if /packages version does not have MT but the system distribution
# does, it will incorrectly use the system version. It would be a good idea
# to periodically check which boost libs the programs are linking to by running
# ldd on them...
set(Boost_USE_MULTITHREADED OFF)

###
### Find Packages
###
find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})

find_package(Octave)
if (OCTAVE_FOUND)
  include_directories(${OCTAVE_INCLUDE_DIRS})
  link_directories(${OCTAVE_LIBRARY_DIRS})
endif()

###
### Set Options
###
option(BINARY_DIR_INSTALL "Install in the build directory" OFF)
option(CMAKE_INSTALL_RPATH_USE_LINK_PATH "Use the link path as the rpath in the install" ON)

# ENABLE_OCTAVE_SUPPORT
if (OCTAVE_FOUND)
  option(ENABLE_OCTAVE_SUPPORT "Build the MVP with octave support" ON)
else()
  option(ENABLE_OCTAVE_SUPPORT "Build the MVP with octave support" OFF)
endif()

# If MFILE_INSTALL_PREFIX, OCTFILE_INSTALL_PREFIX
if (ENABLE_OCTAVE_SUPPORT)
  set(MFILE_INSTALL_PREFIX ${OCTAVE_MFILE_DIR} CACHE PATH "Install prefix for octave m-files")
  set(OCTFILE_INSTALL_PREFIX ${OCTAVE_OCTFILE_DIR} CACHE PATH "Install prefix for octive oct-files")

  mark_as_advanced(CLEAR FORCE MFILE_INSTALL_PREFIX
                               OCTFILE_INSTALL_PREFIX)
else()
  set(MFILE_INSTALL_PREFIX "NOTFOUND" CACHE PATH "Install prefix for octave m-files")
  set(OCTFILE_INSTALL_PREFIX "NOTFOUND" CACHE PATH "Install prefix for octave oct-files")

  mark_as_advanced(FORCE MFILE_INSTALL_PREFIX
                         OCTFILE_INSTALL_PREFIX)
endif()

# Force settings if BINARY_DIR_INSTALL, otherwise make them available
if (BINARY_DIR_INSTALL)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR} CACHE PATH "Install prefix" FORCE)

  if (ENABLE_OCTAVE_SUPPORT)
    set(MFILE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/share/mvp/m CACHE PATH "Install prefix for octave m-files" FORCE)
    set(OCTFILE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX}/share/mvp/oct CACHE PATH "Install prefix for octave oct-files" FORCE)
  endif()

  mark_as_advanced(FORCE CMAKE_INSTALL_PREFIX
                         MFILE_INSTALL_PREFIX
                         OCTFILE_INSTALL_PREFIX)
else()
  mark_as_advanced(CLEAR FORCE CMAKE_INSTALL_PREFIX)

  if (ENABLE_OCTAVE_SUPPORT)
    mark_as_advanced(CLEAR FORCE MFILE_INSTALL_PREFIX
                                 OCTFILE_INSTALL_PREFIX)
  endif()
endif()

###
### Testing
###
option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
  enable_testing()
endif()

###
### Add subdirs 
###
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)
add_subdirectory(src)
