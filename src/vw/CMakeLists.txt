if (ENABLE_OCTAVE_SUPPORT)
  add_subdirectory(Octave)
endif()

# Generate thirdparty vw_protobuf and install
file(GLOB_RECURSE vw_protos RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf/*.proto)
PROTOBUF_GENERATE(IPATHS ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf
                  OUTPUT ${CMAKE_BINARY_DIR}/src
                  PROTOS ${vw_protos})
