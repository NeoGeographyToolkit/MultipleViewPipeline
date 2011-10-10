# Find VW packages
find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

add_executable(orbitfoot orbitfoot.cc)
target_link_libraries(orbitfoot ${VW_LIBRARIES})

add_executable(backproj_test backproj_test.cc)
target_link_libraries(backproj_test ${VW_LIBRARIES})

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
