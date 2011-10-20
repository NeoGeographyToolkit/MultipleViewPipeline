# Hack to include VW's proto files
set(_protobuf_include_path -I ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS MVPJobRequest.proto
                                            OrbitalImageFileDescriptor.proto
                                            MVPAlgorithmSettings.proto)

set(MVP_HDRS MVPAlgorithm.h
             MVPJob.h
             MVPWorkspace.h
             OrbitalImageFootprint.h
             ${CMAKE_CURRENT_BINARY_DIR}/Config.h
             ${PROTO_HDRS})
set(MVP_SRCS ${PROTO_SRCS})

add_library(mvp ${MVP_SRCS})

add_executable(mvp_solo mvp_solo.cc)
target_link_libraries(mvp_solo mvp
                               ${PROTOBUF_LIBRARIES}
                               ${VW_LIBRARIES})
#set_target_properties(mvp_solo PROPERTIES COMPILE_FLAGS "-DTEST_SRCDIR\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"")

configure_file(Config.h.in Config.h)

install(TARGETS mvp_solo DESTINATION bin)
install(FILES ${MVP_HDRS} DESTINATION include/mvp)

if (ENABLE_OCTAVE_SUPPORT)
  add_subdirectory(octave)
endif()

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
