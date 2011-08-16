find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)

find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

add_executable(TestOrbitFoot TestOrbitFoot.cxx)
target_link_libraries(TestOrbitFoot ${GTEST_BOTH_LIBRARIES}
                                    ${VW_LIBRARY}
                                    ${VW_CARTOGRAPHY_LIBRARY}
                                    ${VW_CAMERA_LIBRARY})

add_test(AllTestsInOrbitFoot TestOrbitFoot)
#GTEST_ADD_TESTS(TestOrbitFoot "" TestOrbitFoot.cxx)
