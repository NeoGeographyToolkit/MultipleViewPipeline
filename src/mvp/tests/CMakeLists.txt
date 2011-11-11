macro(add_mvp_test test_name)
  add_executable(${test_name} ${test_name}.cxx )

  target_link_libraries(${test_name} mvpcommon
                                     gtest_main
                                     ${GTEST_LIBRARIES}
                                     ${VW_LIBRARIES})
  if (ENABLE_OCTAVE_SUPPORT)
    target_link_libraries(${test_name} ${OCTAVE_LIBRARIES})
  endif()

  set_target_properties(${test_name} PROPERTIES COMPILE_FLAGS 
                        "-DTEST_SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\" -DTEST_OBJDIR=\\\"${CMAKE_CURRENT_BINARY_DIR}\\\"")

  add_test(AllMVPTests ${test_name})
endmacro()

add_mvp_test(TestMVPWorkspace)
add_mvp_test(TestOrbitalImageFootprint)
add_mvp_test(TestOrbitalImageCrop)
add_mvp_test(TestMVPJob)
add_mvp_test(TestConvexPolygon)
