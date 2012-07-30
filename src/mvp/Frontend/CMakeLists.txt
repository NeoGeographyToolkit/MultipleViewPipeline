mvp_module(NAME Frontend
           HDRS Session.h
           SRCS Session.cc
           PROTOS SessionDesc.proto)

pyinstall_bin(FILES mvpadmin.py
              BASEPATH ${CMAKE_BINARY_DIR}/src 
              DESTINATION ${PYTHON_INSTALL_DIR})

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
