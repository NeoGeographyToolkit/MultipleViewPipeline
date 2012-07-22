add_definitions(-DTEST_SRCDIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
add_definitions(-DTEST_OBJDIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS TestProto.proto)
add_library(mvpCore_test ${PROTO_SRCS})

macro(add_mvp_test test_name)
  add_executable(${test_name} ${test_name}.cxx)

  target_link_libraries(${test_name} mvpCore
                                     mvpCore_test
                                     gtest_main
                                     ${GTEST_LIBRARIES}
                                     ${VW_LIBRARIES}
                                     ${PROTOBUF_LIBRARIES}
                                     ${Boost_LIBRARIES})

  if (ENABLE_OCTAVE_SUPPORT)
    target_link_libraries(${test_name} ${OCTAVE_LIBRARIES})
  endif()

  add_test(mvp/${test_name} ${test_name})
endmacro()

add_mvp_test(TestSettingsParser)
