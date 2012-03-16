cmake_minimum_required (VERSION 2.8)
project (MVP)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/modules")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC")

###
### Load configuration
### TODO: Do the right way
if (EXISTS ${CMAKE_BINARY_DIR}/config.cmake)
  include (${CMAKE_BINARY_DIR}/config.cmake)
endif()

###
### Find Packages
###
find_package(VW REQUIRED COMPONENTS Cartography Camera Plate Image Math FileIO Core)
find_package(Boost REQUIRED COMPONENTS program_options)
find_package(Protobuf REQUIRED)
find_package(ZeroMQ REQUIRED)
find_package(Octave)
find_package(Gearman)

###
### Python Protobuffers
###
include(ProtobufGenerate)
# TODO: CACHE variable?
set(PYTHON_INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/share/mvp/python)

# Generate thirdparty vw_protobuf and install
file(GLOB_RECURSE vw_protos RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf/*.proto)
PROTOBUF_GENERATE(PYOUT PROTO_PYS 
                  IPATHS ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf
                  OUTPUT ${CMAKE_BINARY_DIR}/src
                  PROTOS ${vw_protos})
add_custom_target(vw_proto_python ALL DEPENDS ${PROTO_PYS})
install(FILES ${PROTO_PYS} DESTINATION ${PYTHON_INSTALL_DIR})


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

# MFILE_INSTALL_DIR, OCTFILE_INSTALL_DIR
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
include_directories(${ZEROMQ_INCLUDE_DIRS})

if (ENABLE_OCTAVE_SUPPORT)
  include_directories(${OCTAVE_INCLUDE_DIRS})
  link_directories(${OCTAVE_LIBRARY_DIRS})
endif()

if (ENABLE_GEARMAN_SUPPORT)
  include_directories(${GEARMAN_INCLUDE_DIRS})
endif() 

add_subdirectory(src)
