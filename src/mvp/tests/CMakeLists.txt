include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)
include_directories(${CMAKE_SOURCE_DIR}/src)

find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})

find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

add_executable(TestMVPWorkspace TestMVPWorkspace.cxx 
                             ${CMAKE_SOURCE_DIR}/src/test/test_main.cc)
target_link_libraries(TestMVPWorkspace ${GTEST_LIBRARIES}
                                    ${VW_LIBRARY}
                                    ${VW_CARTOGRAPHY_LIBRARY}
                                    ${VW_CAMERA_LIBRARY})
set_target_properties(TestMVPWorkspace PROPERTIES COMPILE_FLAGS 
                      "-DTEST_SRCDIR=\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\" -DTEST_DSTDIR=\\\"${CMAKE_CURRENT_BINARY_DIR}\\\"")

add_test(AllMVPTests TestMVPWorkspace)
