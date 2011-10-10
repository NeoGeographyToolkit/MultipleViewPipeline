find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

macro(add_vwoctave_test test_name)
  add_executable(${test_name} ${test_name}.cxx 
                               ${CMAKE_SOURCE_DIR}/src/test/test_main.cc)
  target_link_libraries(${test_name} mvp
                                     ${GTEST_LIBRARIES}
                                     ${VW_LIBRARIES}
                                     ${OCTAVE_LIBRARIES})
  set_target_properties(${test_name} PROPERTIES COMPILE_FLAGS 
                        "-DTEST_SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\" -DTEST_OBJDIR=\\\"${CMAKE_CURRENT_BINARY_DIR}\\\"")

  add_test(AllVWOctaveTests ${test_name})
endmacro()

add_vwoctave_test(TestConversions)
