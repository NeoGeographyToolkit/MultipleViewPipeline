include(AddOctfile)

# Find VW packages
find_package(VW REQUIRED)
include_directories(${VW_INCLUDE_DIRS})

# Add octave's libraries
find_package(Octave REQUIRED)
include_directories(${OCTAVE_INCLUDE_DIRS})
link_directories(${OCTAVE_LIBRARY_DIRS})

# Add targets
if (VW_Cartography_FOUND)
  add_octfile(imread_vw imread_vw.cc)
  target_link_libraries(imread_vw ${VW_LIBRARIES} ${VW_Cartography_LIBRARY})
endif()

if (VW_Camera_FOUND)
  add_octfile(loadcam_vw loadcam_vw.cc)
  target_link_libraries(loadcam_vw ${VW_LIBRARIES} ${VW_Camera_LIBRARY})
endif()
