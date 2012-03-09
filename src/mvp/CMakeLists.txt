# Hack to include VW's proto files
set(_protobuf_include_path -I ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS MVPJobRequest.proto
                                            OrbitalImageFileDescriptor.proto
                                            MVPUserSettings.proto
                                            MVPAlgorithmOptions.proto)

set(MVP_HDRS MVPJob.h
             MVPWorkspace.h
             OrbitalImageFootprint.h
             ConvexPolygon.h
             ${CMAKE_CURRENT_BINARY_DIR}/Config.h
             ${PROTO_HDRS})
set(MVP_SRCS ${PROTO_SRCS})

add_library(mvpcommon ${MVP_SRCS})

add_executable(mvp mvp.cc)
target_link_libraries(mvp mvpcommon
                          ${PROTOBUF_LIBRARIES}
                          ${VW_LIBRARIES}
                          ${Boost_LIBRARIES})

add_executable(mvp_dumpjob mvp_dumpjob.cc)
target_link_libraries(mvp_dumpjob mvpcommon
                                  ${PROTOBUF_LIBRARIES}
                                  ${VW_LIBRARIES}
                                  ${Boost_LIBRARIES})

add_executable(mvp_worker mvp_worker.cc)
target_link_libraries(mvp_worker mvpcommon
                                 ${PROTOBUF_LIBRARIES}
                                 ${VW_LIBRARIES}
                                 ${Boost_LIBRARIES})

if (ENABLE_OCTAVE_SUPPORT)
  target_link_libraries(mvp ${OCTAVE_LIBRARIES})
  target_link_libraries(mvp_dumpjob ${OCTAVE_LIBRARIES})
  target_link_libraries(mvp_worker ${OCTAVE_LIBRARIES})
endif()

if (ENABLE_GEARMAN_SUPPORT)
  target_link_libraries(mvp ${GEARMAN_LIBRARIES})
  target_link_libraries(mvp_dumpjob ${GEARMAN_LIBRARIES})
  target_link_libraries(mvp_worker ${GEARMAN_LIBRARIES})
endif()

configure_file(Config.h.in Config.h)

install(TARGETS mvp mvp_dumpjob mvp_worker DESTINATION bin)
install(FILES ${MVP_HDRS} DESTINATION include/mvp)

if (ENABLE_OCTAVE_SUPPORT)
  add_subdirectory(octave)
endif()

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
