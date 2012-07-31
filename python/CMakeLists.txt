include (PythonHelpers)

pyinstall_bin(mvpadmin.py)

# Make / Install protobuf python modules
build_pyproto_tree(PROTO_TREE ${CMAKE_SOURCE_DIR}/src)
build_pyproto_tree(PROTO_TREE ${CMAKE_SOURCE_DIR}/thirdparty/vw_protobuf)

PROTOBUF_GENERATE(PYOUT PROTO_PYS
                  IPATHS ${CMAKE_BINARY_DIR}/python
                  OUTPUT ${CMAKE_BINARY_DIR}/python
                  PROTOS ${PROTO_TREE})

add_custom_target(pyProto ALL DEPENDS ${PROTO_TREE} ${PROTO_PYS})

pyinstall(FILES ${PROTO_PYS} 
          BASEPATH ${CMAKE_BINARY_DIR}/python
          DESTINATION ${PYTHON_INSTALL_DIR})
