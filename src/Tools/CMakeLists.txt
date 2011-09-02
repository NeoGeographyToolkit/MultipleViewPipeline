# Find VW packages
find_package(VW REQUIRED)
include_directories(${VW_INCLUDE_DIRS})

find_package(Boost REQUIRED COMPONENTS system thread filesystem)
include_directories(${Boost_INCLUDE_DIRS})

if (VW_CARTOGRAPHY_FOUND AND
    VW_CAMERA_FOUND)
  add_executable(gen_synth_scene gen_synth_scene.cc)
  target_link_libraries(gen_synth_scene ${VW_CORE_LIBRARY}
                                        ${VW_MATH_LIBRARY}
                                        ${VW_FILEIO_LIBRARY}
                                        ${VW_IMAGE_LIBRARY}
                                        ${VW_CARTOGRAPHY_LIBRARY} 
                                        ${VW_CAMERA_LIBRARY}
                                        ${Boost_SYSTEM_LIBRARY}
                                        ${Boost_THREAD_LIBRARY}
                                        ${Boost_FILESYSTEM_LIBRARY})
endif()

add_executable(vwconvert vwconvert.cc)
target_link_libraries(vwconvert ${VW_CORE_LIBRARY}
                                ${VW_MATH_LIBRARY}
                                ${VW_FILEIO_LIBRARY}
                                ${VW_IMAGE_LIBRARY}
                                ${Boost_SYSTEM_LIBRARY})

if (VW_CAMERA_FOUND)
  add_executable(print_pinhole print_pinhole.cc)
  target_link_libraries(print_pinhole ${VW_CORE_LIBRARY}
                                      ${VW_MATH_LIBRARY}
                                      ${VW_FILEIO_LIBRARY}
                                      ${VW_IMAGE_LIBRARY}
                                      ${VW_CAMERA_LIBRARY}
                                      ${Boost_SYSTEM_LIBRARY})
endif()
