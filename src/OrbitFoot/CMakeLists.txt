# Find VW packages
find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

add_executable(orbitfoot orbitfoot.cc)
target_link_libraries(orbitfoot ${VW_LIBRARY}
                                ${VW_CARTOGRAPHY_LIBRARY} 
                                ${VW_CAMERA_LIBRARY}
                                ${VW_PLATE_LIBRARY})
