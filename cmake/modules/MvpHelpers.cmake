function(mvp_module)
  set(options)
  set(oneValueArgs NAME)
  set(multiValueArgs HDRS SRCS PROTOS)
  cmake_parse_arguments(mvp_module "${options}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

  if (mvp_module_PROTOS)
    PROTOBUF_GENERATE(CPPOUT PROTO_SRCS 
                      HPPOUT PROTO_HDRS
                      PYOUT PROTO_PYS 
                      IPATHS ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf
                      OUTPUT ${CMAKE_BINARY_DIR}/src 
                      PROTOS ${mvp_module_PROTOS})

    # Build python protos
    add_custom_target(mvp${mvp_module_NAME}Python ALL DEPENDS ${PROTO_PYS})

    # Install python executables / support
    INSTALL_PROTOBUF_PYS(FILES ${PROTO_PYS} 
                         BASEPATH ${CMAKE_BINARY_DIR}/src
                         DESTINATION ${PYTHON_INSTALL_DIR})
   
    set(mvp_module_HDRS ${mvp_module_HDRS} ${PROTO_HDRS}) 
    set(mvp_module_SRCS ${mvp_module_SRCS} ${PROTO_SRCS})
  endif()

  # Build library
  add_library(mvp${mvp_module_NAME} SHARED ${mvp_module_SRCS})

  # Install headers
  install(FILES ${mvp_module_HDRS} DESTINATION include/mvp/${mvp_module_NAME})

  # Install library
  install(TARGETS mvp${mvp_module_NAME} DESTINATION lib)
endfunction()

function(mvp_test)
  add_definitions(-DTEST_SRCDIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")
  add_definitions(-DTEST_OBJDIR=\"${CMAKE_CURRENT_BINARY_DIR}\")

  add_executable(${test_name} ${test_name}.cxx)

  target_link_libraries(${test_name} mvpCore
                                     gtest_main
                                     ${GTEST_LIBRARIES}
                                     ${VW_LIBRARIES}
                                     ${PROTOBUF_LIBRARIES}
                                     ${Boost_LIBRARIES})

  if (ENABLE_OCTAVE_SUPPORT)
    target_link_libraries(${test_name} ${OCTAVE_LIBRARIES})
  endif()

  add_test(mvp/${test_name} ${test_name})
endfunction()
