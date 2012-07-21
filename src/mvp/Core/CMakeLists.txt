set(MVP_PROTOS GlobalSettings.proto)

PROTOBUF_GENERATE(CPPOUT PROTO_SRCS 
                  HPPOUT PROTO_HDRS
                  PYOUT PROTO_PYS 
                  IPATHS ${CMAKE_SOURCE_DIR}/src ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf
                  OUTPUT ${CMAKE_BINARY_DIR}/src 
                  PROTOS ${MVP_PROTOS})

set(MVP_HDRS SettingsParser.h
             ${PROTO_HDRS})

set(MVP_SRCS SettingsParser.cc
             ${PROTO_SRCS})

# Build python libs
add_custom_target(mvpCorePython ALL DEPENDS ${PROTO_PYS})

# Build library
add_library(mvpCore ${MVP_SRCS})

# Install headers
install(FILES ${MVP_HDRS} DESTINATION include/mvp/Core)

# Install python executables / support
INSTALL_PROTOBUF_PYS(FILES ${PROTO_PYS} 
                     BASEPATH ${CMAKE_BINARY_DIR}/src
                     DESTINATION ${PYTHON_INSTALL_DIR})

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
