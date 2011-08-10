
if (BUILD_OCTAVEVW)
  add_subdirectory(OctaveVW)
endif()

if (NOT OCTAVE_FOUND)
  message(WARNING "blah")
endif()
