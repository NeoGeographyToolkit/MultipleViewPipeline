# Find VW packages
include_directories(/byss/khusmann/projects/build/include)
include_directories(/byss/packages/boost/include)
link_directories(/byss/khusmann/projects/build/lib)

find_package(Octave REQUIRED)

if (NOT OCTAVE_FOUND)
  message(SEND_ERROR "octave is required to build OctaveVW!")
endif()

include_directories(${OCTAVE_INCLUDE_DIRS})
link_directories(${OCTAVE_LINK_DIRS})

add_library(imread_vw MODULE imread_vw.cc)
target_link_libraries(imread_vw vw vwCartography ${OCTAVE_LIBRARY})

add_library(loadcam_vw MODULE loadcam_vw.cc)
target_link_libraries(loadcam_vw vw vwCamera ${OCTAVE_LIBRARY})

set(OCTFILE_LIST imread_vw loadcam_vw)

set_target_properties(${OCTFILE_LIST} PROPERTIES
                      LINK_FLAGS "${OCTAVE_LINK_FLAGS}"
                      COMPILE_FLAGS "${OCTAVE_CXXFLAGS}"
                      PREFIX ""
                      SUFFIX ".oct"
                     )
