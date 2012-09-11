add_definitions(-DTEST_SRCDIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
add_definitions(-DTEST_OBJDIR=\"${CMAKE_CURRENT_BINARY_DIR}\")
add_definitions(-DTEST_DATADIR=\"${CMAKE_SOURCE_DIR}/src/test/data\")

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS TestProto.proto)

add_library(vwoctave_test ${PROTO_SRCS})

macro(add_vwoctave_test test_name)
  add_executable(${test_name} ${test_name}.cxx)

  target_link_libraries(${test_name} vwoctave_test 
                                     gtest_main
                                     vwOctave
                                     ${VW_LIBRARIES}
                                     ${OCTAVE_LIBRARIES}
                                     ${PROTOBUF_LIBRARIES})

  add_test(vw/Octave/${test_name} ${test_name})
endmacro()

add_vwoctave_test(TestConversions)
