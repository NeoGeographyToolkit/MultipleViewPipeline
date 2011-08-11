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

macro(ADD_OCTFILE _octfile _sources)
  add_library(${_octfile} MODULE ${_sources})
  target_link_libraries(${_octfile} ${OCTAVE_LIBRARIES})

  set_target_properties(${_octfile} PROPERTIES
                        LINK_FLAGS "${OCTAVE_LINK_FLAGS}"
                        COMPILE_FLAGS "${OCTAVE_COMPILE_FLAGS}"
                        PREFIX ""
                        SUFFIX ".oct"
                       )
endmacro()

add_octfile(imread_vw imread_vw.cc)
add_octfile(loadcam_vw loadcam_vw.cc)
