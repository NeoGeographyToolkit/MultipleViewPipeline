include(AddOctfile)

# Find VW packages
include_directories(/byss/khusmann/projects/build/include)
include_directories(/byss/packages/boost/include)
link_directories(/byss/khusmann/projects/build/lib)

# Add octave's libraries
find_package(Octave REQUIRED)
include_directories(${OCTAVE_INCLUDE_DIRS})
link_directories(${OCTAVE_LINK_DIRS})

# Add targets
add_octfile(imread_vw imread_vw.cc)
target_link_libraries(imread_vw vw vwCartography)

add_octfile(loadcam_vw loadcam_vw.cc)
target_link_libraries(loadcam_vw vw vwCamera)
