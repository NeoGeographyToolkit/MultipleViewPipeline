include_directories(${CMAKE_SOURCE_DIR}/src)

# Find VW packages
find_package(VW REQUIRED COMPONENTS Cartography Camera)
include_directories(${VW_INCLUDE_DIRS})

# Add octave's libraries
find_package(Octave REQUIRED)
include_directories(${OCTAVE_INCLUDE_DIRS})
link_directories(${OCTAVE_LIBRARY_DIRS})

# Add targets
add_octfile(imread_vw imread_vw.cc)
target_link_libraries(imread_vw ${VW_LIBRARIES})

add_octfile(loadcam_vw loadcam_vw.cc)
target_link_libraries(loadcam_vw ${VW_LIBRARIES})
