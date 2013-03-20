PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS DummyProto.proto)

mvp_test(TestConversions)

add_library(mvpoctave_test ${PROTO_SRCS})

target_link_libraries(TestConversions mvpoctave_test)
