cmake_minimum_required (VERSION 2.6)
project (MVP)

include(CMakeDependentOption)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")

# TODO:
# This is to avoid the bug http://public.kitware.com/Bug/view.php?id=12442
# Otherwise, if /packages version does not have MT but the system distribution
# does, it will incorrectly use the system version. It would be a good idea
# to periodically check which boost libs the programs are linking to by running
# ldd on them...
# set(Boost_USE_MULTITHREADED OFF)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall")

###
### Find Packages
###
find_package(VW REQUIRED COMPONENTS Cartography Camera Plate Image Math FileIO Core)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Protobuf REQUIRED)
find_package(Octave)
find_package(Gearman)

###
### Set Options
###
option(BINARY_DIR_INSTALL "Install in the build directory" OFF)
option(CMAKE_INSTALL_RPATH_USE_LINK_PATH "Use the link path as the rpath in the install" ON)

# ENABLE_GEARMAN_SUPPORT
if (GEARMAN_FOUND)
  option(ENABLE_GEARMAN_SUPPORT "Build the MVP with gearman support" ON)
else()
  option(ENABLE_GEARMAN_SUPPORT "Build the MVP with gearman support" OFF)
endif()

# ENABLE_OCTAVE_SUPPORT
if (OCTAVE_FOUND)
  option(ENABLE_OCTAVE_SUPPORT "Build the MVP with octave support" ON)
else()
  option(ENABLE_OCTAVE_SUPPORT "Build the MVP with octave support" OFF)
endif()

# If MFILE_INSTALL_DIR, OCTFILE_INSTALL_DIR
if (ENABLE_OCTAVE_SUPPORT)
  set(MFILE_INSTALL_DIR ${OCTAVE_MFILE_DIR}/mvp CACHE PATH "Install prefix for octave m-files")
  set(OCTFILE_INSTALL_DIR ${OCTAVE_OCTFILE_DIR} CACHE PATH "Install prefix for octive oct-files")

  mark_as_advanced(CLEAR FORCE MFILE_INSTALL_DIR
                               OCTFILE_INSTALL_DIR)
else()
  set(MFILE_INSTALL_DIR "NOTFOUND" CACHE PATH "Install prefix for octave m-files")
  set(OCTFILE_INSTALL_DIR "NOTFOUND" CACHE PATH "Install prefix for octave oct-files")

  mark_as_advanced(FORCE MFILE_INSTALL_DIR
                         OCTFILE_INSTALL_DIR)
endif()

# Force settings if BINARY_DIR_INSTALL, otherwise make them available
if (BINARY_DIR_INSTALL)
  set(CMAKE_INSTALL_PREFIX ${CMAKE_BINARY_DIR}/usr CACHE PATH "Install prefix" FORCE)

  if (ENABLE_OCTAVE_SUPPORT)
    set(MFILE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/mvp/m CACHE PATH "Install prefix for octave m-files" FORCE)
    set(OCTFILE_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/mvp/oct CACHE PATH "Install prefix for octave oct-files" FORCE)
  endif()

  mark_as_advanced(FORCE CMAKE_INSTALL_PREFIX
                         MFILE_INSTALL_DIR
                         OCTFILE_INSTALL_DIR)
else()
  mark_as_advanced(CLEAR FORCE CMAKE_INSTALL_PREFIX)

  if (ENABLE_OCTAVE_SUPPORT)
    mark_as_advanced(CLEAR FORCE MFILE_INSTALL_DIR
                                 OCTFILE_INSTALL_DIR)
  endif()
endif()

###
### Testing
###
option(BUILD_TESTS "Build tests" OFF)

if (BUILD_TESTS)
  enable_testing()
  include_directories(${CMAKE_SOURCE_DIR}/thirdparty/gtest/include)
endif()

###
### Add include paths and subdirs to build 
###
include_directories(${CMAKE_SOURCE_DIR}/src)
include_directories(${CMAKE_BINARY_DIR}/src)
include_directories(${VW_INCLUDE_DIRS})
include_directories(${PROTOBUF_INCLUDE_DIRS})

if (ENABLE_OCTAVE_SUPPORT)
  include_directories(${OCTAVE_INCLUDE_DIRS})
  link_directories(${OCTAVE_LIBRARY_DIRS})
endif()

add_subdirectory(src)
