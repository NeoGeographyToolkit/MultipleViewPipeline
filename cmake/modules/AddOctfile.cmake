macro(ADD_OCTFILE _octfile _sources)
  add_library(${_octfile} MODULE ${_sources})
  target_link_libraries(${_octfile} ${OCTAVE_LIBRARIES})

  set_target_properties(${_octfile} PROPERTIES
                        LINK_FLAGS "${OCTAVE_LINK_FLAGS}"
                        COMPILE_FLAGS "${OCTAVE_COMPILE_FLAGS}"
                        PREFIX ""
                        SUFFIX ".oct"
                       )
endmacro()


