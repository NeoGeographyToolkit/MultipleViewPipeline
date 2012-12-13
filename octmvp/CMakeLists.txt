cmake_minimum_required (VERSION 2.8)
project (OCTMVP)

set(MVP_DIR ${CMAKE_SOURCE_DIR}/..)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${MVP_DIR}/cmake/modules")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -fPIC -O3")

find_package(Octave)
include_directories(${OCTAVE_INCLUDE_DIRS})
link_directories(${OCTAVE_LIBRARY_DIRS})

configure_file(init_mvp.m.in init_mvp.m)
configure_file(test_mvp.m.in test_mvp.m)

add_subdirectory(src)

enable_testing()
