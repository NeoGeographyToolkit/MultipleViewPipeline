# Hack to include VW's proto files
set(_protobuf_include_path -I ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS MVPJobRequest.proto
                                            OrbitalImageFileDesc.proto
                                            MVPAlgorithmSettings.proto
                                            TileDescriptor.proto)


add_library(mvp ${PROTO_SRCS})

add_executable(mvp_solo mvp_solo.cc)
target_link_libraries(mvp_solo mvp
                               ${PROTOBUF_LIBRARIES}
                               ${VW_LIBRARIES})

set(MVP_HDRS MVPAlgorithm.h
             MVPTileProcessor.h
             MVPWorkspace.h
             OrbitalImageFile.h
             ${CMAKE_CURRENT_BINARY_DIR}/Config.h
             ${PROTO_HDRS})

configure_file(Config.h.in Config.h)

install(TARGETS mvp_solo DESTINATION bin)
install(FILES ${MVP_HDRS} DESTINATION include/mvp)

if (ENABLE_OCTAVE_SUPPORT)
  add_subdirectory(octave)
endif()

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
