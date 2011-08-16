# Find VW packages
find_package(VW REQUIRED)
include_directories(${VW_INCLUDE_DIRS})

if (VW_CARTOGRAPHY_FOUND AND
    VW_CAMERA_FOUND)
  add_executable(gen_synth_scene gen_synth_scene.cc)
  target_link_libraries(gen_synth_scene ${VW_LIBRARY}
                                        ${VW_CARTOGRAPHY_LIBRARY} 
                                        ${VW_CAMERA_LIBRARY})
endif()

add_executable(vwconvert vwconvert.cc)
target_link_libraries(vwconvert ${VW_LIBRARIES})

if (VW_CAMERA_FOUND)
  add_executable(print_pinhole print_pinhole.cc)
  target_link_libraries(print_pinhole ${VW_LIBRARY}
                                      ${VW_CAMERA_LIBRARY})
endif()
