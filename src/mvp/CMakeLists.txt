# Hack to include VW's proto files
set(_protobuf_include_path -I ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf)

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS MVPJobRequest.proto
                                            OrbitalImageFileDescriptor.proto
                                            MVPUserSettings.proto)

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
if (ENABLE_OCTAVE_SUPPORT)
  target_link_libraries(mvp ${OCTAVE_LIBRARIES})
endif()
#set_target_properties(mvp_solo PROPERTIES COMPILE_FLAGS "-DTEST_SRCDIR\\\"${CMAKE_CURRENT_SOURCE_DIR}\\\"")

configure_file(Config.h.in Config.h)

install(TARGETS mvp DESTINATION bin)
install(FILES ${MVP_HDRS} DESTINATION include/mvp)

if (ENABLE_OCTAVE_SUPPORT)
  add_subdirectory(octave)
endif()

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
