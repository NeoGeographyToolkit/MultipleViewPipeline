function(PROTOBUF_GENERATE)
  set(options)
  set(oneValueArgs CPPOUT HPPOUT PYOUT OUTPUT)
  set(multiValueArgs IPATHS PROTOS)
  cmake_parse_arguments(PROTOBUF_GENERATE "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT(PROTOBUF_GENERATE_CPPOUT AND PROTOBUF_GENERATE_HPPOUT))
    if (PROTOBUF_GENERATE_CPPOUT OR PROTOBUF_GENERATE_HPPOUT)
      message(SEND_ERROR "Error: PROTOBUF_GENERATE missing CPPOUT or HPPOUT")
      return()
    endif()
  endif()

  if (NOT PROTOBUF_GENERATE_PROTOS)
    message(SEND_ERROR "Error: No .proto files specified")
  endif()

  if (NOT PROTOBUF_GENERATE_OUTPUT)
    set(PROTOBUF_GENERATE_OUTPUT ${CMAKE_CURRENT_BINARY_DIR})
  endif()

  if (NOT PROTOBUF_GENERATE_IPATHS)
    set(PROTOBUF_GENERATE_IPATHS ${CMAKE_CURRENT_SOURCE_DIR})
  endif()

  foreach(DIR ${PROTOBUF_GENERATE_IPATHS})
    list(APPEND _include_list -I${DIR})
  endforeach()

  foreach(FIL ${PROTOBUF_GENERATE_PROTOS})
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    # protoc will generate files relative to the first -I on the
    # path that the .proto file is a child of. So, we
    # find that one so we know where the generated files will end up
    foreach(PAT ${PROTOBUF_GENERATE_IPATHS})
      file(RELATIVE_PATH _proto_rel ${PAT} ${ABS_FIL})
      if (NOT _proto_rel MATCHES "^\\.\\.")
        get_filename_component(_proto_rel_path ${_proto_rel} PATH)
        break()
      endif()  
    endforeach()

    if (NOT _proto_rel_path)
      message(SEND_ERROR "Error: ${FIL} is not in a subdir of ${PROTOBUF_GENERATE_IPATHS}")
    endif()

    set(_output_stem "${PROTOBUF_GENERATE_OUTPUT}/${_proto_rel_path}/${FIL_WE}")

    if (PROTOBUF_GENERATE_CPPOUT) 
      list(APPEND ${PROTOBUF_GENERATE_CPPOUT} "${_output_stem}.pb.cc")
      list(APPEND ${PROTOBUF_GENERATE_HPPOUT} "${_output_stem}.pb.h")

      add_custom_command(
        OUTPUT "${_output_stem}.pb.cc"
               "${_output_stem}.pb.h"
        COMMAND  ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --cpp_out  ${PROTOBUF_GENERATE_OUTPUT} ${_include_list} ${ABS_FIL}
        DEPENDS ${ABS_FIL}
        COMMENT "Running C++ protocol buffer compiler on ${FIL}"
        VERBATIM )
    endif()

    if (PROTOBUF_GENERATE_PYOUT)
      list(APPEND ${PROTOBUF_GENERATE_PYOUT} "${_output_stem}_pb2.py")
      add_custom_command(
        OUTPUT "${_output_stem}_pb2.py"
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --python_out ${PROTOBUF_GENERATE_OUTPUT} ${_include_list} ${ABS_FIL}
        DEPENDS ${ABS_FIL}
        COMMENT "Running Python protocol buffer compiler on ${FIL}"
        VERBATIM )
    endif()
  endforeach()

  if (PROTOBUF_GENERATE_CPPOUT)
    set_source_files_properties(${${PROTOBUF_GENERATE_CPPOUT}} ${${PROTOBUF_GENERATE_HPPOUT}} PROPERTIES GENERATED TRUE)
    set(${PROTOBUF_GENERATE_CPPOUT} ${${PROTOBUF_GENERATE_CPPOUT}} PARENT_SCOPE)
    set(${PROTOBUF_GENERATE_HPPOUT} ${${PROTOBUF_GENERATE_HPPOUT}} PARENT_SCOPE)
  endif()

  if (PROTOBUF_GENERATE_PYOUT)
    set_source_files_properties(${${PROTOBUF_GENERATE_PYOUT}} PROPERTIES GENERATED TRUE)
    set(${PROTOBUF_GENERATE_PYOUT} ${${PROTOBUF_GENERATE_PYOUT}} PARENT_SCOPE)
  endif()
endfunction()
