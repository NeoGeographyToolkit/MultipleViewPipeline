# Add targets
add_octfile(imread_vw imread_vw.cc)
target_link_libraries(imread_vw ${VW_LIBRARIES})

add_octfile(loadcam_vw loadcam_vw.cc)
target_link_libraries(loadcam_vw ${VW_LIBRARIES})

install(TARGETS imread_vw
                loadcam_vw
                DESTINATION ${OCTFILE_INSTALL_PREFIX})
install(FILES Conversions.h DESTINATION include/vw/Octave)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
