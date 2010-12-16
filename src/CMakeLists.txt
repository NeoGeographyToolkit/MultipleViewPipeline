cmake_minimum_required (VERSION 2.6)
project (AMVP)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/modules")

include_directories(~/projects/VisionWorkbench/build/include)
link_directories(~/projects/VisionWorkbench/build/lib)

add_executable(gen_synth_scene gen_synth_scene.cc)
target_link_libraries(gen_synth_scene vw vwCartography vwCamera)

find_package(Octave REQUIRED)

include_directories(${OCTAVE_INCLUDE_DIRS})
link_directories(${OCTAVE_LINK_DIRS})

add_library(imread_vw SHARED imread_vw.cc)
target_link_libraries(imread_vw vw vwCartography)

add_library(loadcam_vw SHARED loadcam_vw.cc)
target_link_libraries(loadcam_vw vw vwCamera)

set(OCTFILE_LIST imread_vw loadcam_vw)

target_link_libraries(${OCTFILE_LIST} ${OCTAVE_LIBRARY})
set_target_properties(${OCTFILE_LIST} PROPERTIES
                      LINK_FLAGS ${OCTAVE_LINK_FLAGS}
                      COMPILE_FLAGS ${OCTAVE_CXXFLAGS}
                      PREFIX ""
                      SUFFIX .oct
                     )
