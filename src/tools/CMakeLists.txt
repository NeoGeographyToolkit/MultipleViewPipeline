if (VW_CARTOGRAPHY_FOUND AND VW_CAMERA_FOUND)
  add_executable(gen_synth_scene gen_synth_scene.cc)
  target_link_libraries(gen_synth_scene ${VW_LIBRARIES})
endif()

add_executable(vwconvert vwconvert.cc)
target_link_libraries(vwconvert ${VW_LIBRARIES})

if (VW_CAMERA_FOUND)
  add_executable(print_pinhole print_pinhole.cc)
  target_link_libraries(print_pinhole ${VW_LIBRARIES})
endif()
