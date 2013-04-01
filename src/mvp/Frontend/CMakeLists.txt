mvp_module(NAME Frontend
           DEPS mvpCore
                mvpPipeline
                mvpImage
           HDRS StatusTable.h
                ZmqHelpers.h
                ZmqWorkerHelper.h
                ZmqServerHelper.h
           SRCS StatusTable.cc
                ZmqHelpers.cc
                ZmqWorkerHelper.cc
                ZmqServerHelper.cc
           PROTOS Messages.proto)

#TODO: This can be removed when include <test/Helpers.h> is removed from mvpworker.cc
add_definitions(-DTEST_SRCDIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
add_definitions(-DTEST_OBJDIR=\"${CMAKE_CURRENT_BINARY_DIR}\")
add_definitions(-DTEST_DATADIR=\"${CMAKE_SOURCE_DIR}/src/test/data\")
###### end

mvp_executable(mvpd mvpd.cc)
mvp_executable(mvpworker mvpworker.cc)
mvp_executable(mvpdumpjob mvpdumpjob.cc)

add_octfile(loadjobfile loadjobfile.cc)
target_link_libraries(loadjobfile mvpOctave mvpAlgorithm mvpPipeline)

install(TARGETS mvpd mvpworker DESTINATION bin)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
