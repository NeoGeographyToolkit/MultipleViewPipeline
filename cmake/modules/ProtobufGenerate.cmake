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
        COMMENT "Running C++ protocol buffer compiler on ${FIL_WE}.proto"
        VERBATIM )
    endif()

    if (PROTOBUF_GENERATE_PYOUT)
      list(APPEND ${PROTOBUF_GENERATE_PYOUT} "${_output_stem}_pb2.py")
      add_custom_command(
        OUTPUT "${_output_stem}_pb2.py"
        COMMAND ${PROTOBUF_PROTOC_EXECUTABLE}
        ARGS --python_out ${PROTOBUF_GENERATE_OUTPUT} ${_include_list} ${ABS_FIL}
        DEPENDS ${ABS_FIL}
        COMMENT "Running Python protocol buffer compiler on ${FIL_WE}.proto"
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

function(INSTALL_PROTOBUF_PYS)
  set(options)
  set(oneValueArgs BASEPATH DESTINATION)
  set(multiValueArgs FILES)
  cmake_parse_arguments(INSTALL_PROTOBUF_PYS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT INSTALL_PROTOBUF_PYS_BASEPATH)
    message(SEND_ERROR "Error: BASEPATH not specified")
  endif()

  if (NOT INSTALL_PROTOBUF_PYS_DESTINATION)
    message(SEND_ERROR "Error: DESTINATION not specified")
  endif()

  if (NOT INSTALL_PROTOBUF_PYS_FILES)
    message(SEND_ERROR "Error: FILES not specified")
  endif()

  set(_rel_path_list)
  foreach(PPY ${INSTALL_PROTOBUF_PYS_FILES})
    file(RELATIVE_PATH _rel ${INSTALL_PROTOBUF_PYS_BASEPATH} ${PPY})
    get_filename_component(_rel_path ${_rel} PATH)

    # Make module path lowercase (python modules should be lowercase)
    string(TOLOWER ${_rel_path} _rel_path)
 
    install(FILES ${PPY} DESTINATION ${INSTALL_PROTOBUF_PYS_DESTINATION}/${_rel_path})
    list(APPEND _rel_path_list ${_rel_path})
  endforeach()

  # In order for python to see the installed py files as a valid module 
  # there needs to be an __init__.py in each dir that the pys are installed
  # into. So we build a list of those dirs here.
  set(_init_list)
  list(REMOVE_DUPLICATES _rel_path_list)
  foreach(_rel_path ${_rel_path_list})
    # Split the path into a list by folder
    string(REGEX REPLACE "/" ";" _path_split ${_rel_path})

    set(_running_split)
    foreach(_part ${_path_split})
      # One folder at a time piece it together
      set(_running_split ${_running_split} ${_part})
      string(REGEX REPLACE ";" "/" _running_path "${_running_split}")

      # Add to _init_list if it's not there already
      list(FIND _init_list ${_running_path} _contains_already)
      if (${_contains_already} EQUAL -1)
        list(APPEND _init_list ${_running_path})
      endif()
    endforeach()
  endforeach()

  # Now, for each init path, add an __init__.py
  foreach(_rel_path ${_init_list})
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${INSTALL_PROTOBUF_PYS_DESTINATION}/${_rel_path}/__init__.py)")
  endforeach()
endfunction()
