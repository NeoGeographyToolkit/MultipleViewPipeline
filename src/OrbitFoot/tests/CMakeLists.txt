find_package(GTest REQUIRED)
include_directories(${GTEST_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/..)


add_executable(TestOrbitFoot TestOrbitFoot.cxx)
target_link_libraries(TestOrbitFoot ${GTEST_BOTH_LIBRARIES})

add_test(AllTestsInOrbitFoot TestOrbitFoot)
#GTEST_ADD_TESTS(TestOrbitFoot "" TestOrbitFoot.cxx)
