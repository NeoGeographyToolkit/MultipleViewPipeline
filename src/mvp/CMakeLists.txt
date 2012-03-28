set(MVP_PROTOS MVPJobRequest.proto
               OrbitalImageFileDescriptor.proto
               MVPUserSettings.proto
               MVPAlgorithmOptions.proto
               MVPMessages.proto
               MVPWorkspaceRequest.proto)

PROTOBUF_GENERATE(CPPOUT PROTO_SRCS 
                  HPPOUT PROTO_HDRS
                  PYOUT PROTO_PYS 
                  IPATHS ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf
                  OUTPUT ${CMAKE_BINARY_DIR}/src 
                  PROTOS ${MVP_PROTOS})

set(MVP_HDRS MVPJob.h
             MVPJobImpl.h
             MVPJobImplBase.h
             MVPWorkspace.h
             OrbitalImageFootprint.h
             ConvexPolygon.h
             MVPAlgorithm.h
             MVPTileSeeder.h
             ${CMAKE_CURRENT_BINARY_DIR}/Config.h
             ${PROTO_HDRS})
set(MVP_SRCS ${PROTO_SRCS}
             ConvexPolygon.cc
             OrbitalImageFootprint.cc
             OrbitalImageCrop.cc
             MVPJobImplBase.cc
             MVPJobImpl.cc
             MVPJob.cc
             MVPWorkspace.cc
             MVPTileSeeder.cc
             MVPAlgorithm.cc)

add_library(mvpcommon ${MVP_SRCS})

add_executable(mvpd mvpd.cc)
target_link_libraries(mvpd mvpcommon
                           ${PROTOBUF_LIBRARIES}
                           ${VW_LIBRARIES}
                           ${Boost_LIBRARIES}
                           ${ZEROMQ_LIBRARIES})

add_executable(mvpdumpjob mvpdumpjob.cc)
target_link_libraries(mvpdumpjob mvpcommon
                                 ${PROTOBUF_LIBRARIES}
                                 ${VW_LIBRARIES}
                                 ${Boost_LIBRARIES}
                                 ${ZEROMQ_LIBRARIES})

add_executable(mvpworker mvpworker.cc)
target_link_libraries(mvpworker mvpcommon
                                ${PROTOBUF_LIBRARIES}
                                ${VW_LIBRARIES}
                                ${Boost_LIBRARIES}
                                ${ZEROMQ_LIBRARIES})

if (ENABLE_OCTAVE_SUPPORT)
  target_link_libraries(mvpworker ${OCTAVE_LIBRARIES})
  target_link_libraries(mvpdumpjob ${OCTAVE_LIBRARIES})
endif()

configure_file(Config.h.in Config.h)

# Build mvpadmin
configure_file(mvpadmin.py mvpadmin)
add_custom_target(mvpadmin ALL DEPENDS ${PROTO_PYS})

# Install executables / headers
install(TARGETS mvpd mvpworker mvpdumpjob DESTINATION bin)
install(FILES ${MVP_HDRS} DESTINATION include/mvp)

# Install python executables / support
install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/mvpadmin DESTINATION bin)
INSTALL_PROTOBUF_PYS(FILES ${PROTO_PYS} 
                     BASEPATH ${CMAKE_BINARY_DIR}/src
                     DESTINATION ${PYTHON_INSTALL_DIR})

if (ENABLE_OCTAVE_SUPPORT)
  add_subdirectory(octave)
endif()

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
