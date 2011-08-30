# Find VW packages
find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS System Thread Filesystem)
# TODO: add include_dirs and link dirs?

add_executable(orbitfoot orbitfoot.cc)
target_link_libraries(orbitfoot         ${VW_CORE_LIBRARY}
                                        ${VW_MATH_LIBRARY}
					${VW_FILEIO_LIBRARY}
					${VW_IMAGE_LIBRARY}
                                        ${VW_CARTOGRAPHY_LIBRARY} 
                                        ${VW_CAMERA_LIBRARY}
                                        ${Boost_SYSTEM_LIBRARY}
                                        ${Boost_THREAD_LIBRARY}
                                        ${Boost_FILESYSTEM_LIBRARY}
                                        ${VW_PLATE_LIBRARY})


add_executable(backproj_test backproj_test.cc)
target_link_libraries(backproj_test     ${VW_CORE_LIBRARY}
                                        ${VW_MATH_LIBRARY}
					${VW_FILEIO_LIBRARY}
					${VW_IMAGE_LIBRARY}
                                        ${VW_CARTOGRAPHY_LIBRARY} 
                                        ${VW_CAMERA_LIBRARY}
                                        ${Boost_SYSTEM_LIBRARY}
                                        ${Boost_THREAD_LIBRARY}
                                        ${Boost_FILESYSTEM_LIBRARY}
                                        ${VW_PLATE_LIBRARY})

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
