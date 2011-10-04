include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

macro(add_mvp_test test_name)
  add_executable(${test_name} ${test_name}.cxx 
                               ${CMAKE_SOURCE_DIR}/src/test/test_main.cc)
  target_link_libraries(${test_name} mvp
                                     ${GTEST_LIBRARIES}
                                     ${VW_LIBRARY}
                                     ${VW_CARTOGRAPHY_LIBRARY}
                                     ${VW_CAMERA_LIBRARY})
  set_target_properties(${test_name} PROPERTIES COMPILE_FLAGS 
                        "-DTEST_SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\" -DTEST_OBJDIR=\\\"${CMAKE_CURRENT_BINARY_DIR}\\\"")

  add_test(AllMVPTests ${test_name})
endmacro()

add_mvp_test(TestMVPWorkspace)
add_mvp_test(TestOrbitalImage)

