# Find VW packages
find_package(VW REQUIRED)
include_directories(${VW_INCLUDE_DIRS})

# Find Boost libraries
# find_package(Boost REQUIRED COMPONENTS program_options)

if (VW_Cartography_FOUND AND
    VW_Camera_FOUND)
  add_executable(gen_synth_scene gen_synth_scene.cc)
  target_link_libraries(gen_synth_scene ${VW_LIBRARY}
                                        ${VW_Cartography_LIBRARY} 
                                        ${VW_Camera_LIBRARY})
endif()

add_executable(vwconvert vwconvert.cc)
target_link_libraries(vwconvert ${VW_LIBRARIES})
