# Add octfiles
add_octfile(imread_vw imread_vw.cc)
target_link_libraries(imread_vw ${VW_LIBRARIES})

# Build library
set(vw_octave_SRCS Main.cc
                   Conversions.cc)

add_library(vwOctave SHARED ${vw_octave_SRCS})

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
