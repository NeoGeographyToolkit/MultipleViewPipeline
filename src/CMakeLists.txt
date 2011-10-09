if (BUILD_TOOLS)
  add_subdirectory(Tools)
endif()

if (BUILD_ORBITFOOT)
  add_subdirectory(OrbitFoot)
endif()

add_subdirectory(mvp)
add_subdirectory(vwoctave)
