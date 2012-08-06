mvp_module(NAME Frontend
           HDRS Session.h
                SessionStatus.h
                ZmqHelpers.h
                ZmqWorkerHelper.h
                ZmqServerHelper.h
           SRCS Session.cc
                SessionStatus.cc
                ZmqHelpers.cc
                ZmqWorkerHelper.cc
                ZmqServerHelper.cc
           PROTOS SessionDesc.proto
                  Messages.proto)

mvp_executable(mvpd mvpd.cc)
mvp_executable(mvpworker mvpworker.cc)

install(TARGETS mvpd mvpworker DESTINATION bin)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
