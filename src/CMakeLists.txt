add_subdirectory(mvp)

if (BUILD_VWOCTAVE)
  add_subdirectory(vwoctave)
endif()

if (BUILD_TOOLS)
  add_subdirectory(tools)
endif()
