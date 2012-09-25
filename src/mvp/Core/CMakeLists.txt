mvp_module(NAME Core
           DEPS ${VW_LIBRARIES}
                ${PROTOBUF_LIBRARIES}
                ${Boost_LIBRARIES}
                ${ZEROMQ_LIBRARIES}
           HDRS Settings.h
           SRCS Settings.cc
           PROTOS GlobalSettings.proto)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
