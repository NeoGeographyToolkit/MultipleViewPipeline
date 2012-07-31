function(pyinstall)
  set(options)
  set(oneValueArgs BASEPATH DESTINATION)
  set(multiValueArgs FILES)
  cmake_parse_arguments(pyinstall "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if (NOT pyinstall_BASEPATH)
    message(SEND_ERROR "Error: BASEPATH not specified")
  endif()

  if (NOT pyinstall_DESTINATION)
    message(SEND_ERROR "Error: DESTINATION not specified")
  endif()

  if (NOT pyinstall_FILES)
    message(SEND_ERROR "Error: FILES not specified")
  endif()

  set(_rel_path_list)
  foreach(PPY ${pyinstall_FILES})
    file(RELATIVE_PATH _rel ${pyinstall_BASEPATH} ${PPY})
    get_filename_component(_rel_path ${_rel} PATH)
    install(FILES ${PPY} DESTINATION ${pyinstall_DESTINATION}/${_rel_path})
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
    install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E touch ${pyinstall_DESTINATION}/${_rel_path}/__init__.py)")
  endforeach()
endfunction()

function(pyinstall_bin pyfile)
  configure_file(${pyfile} ${pyfile} @ONLY)
  install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${pyfile} DESTINATION bin)
endfunction()

function(build_pyproto_tree tree root)
  file(GLOB_RECURSE all_protos RELATIVE ${root} ${root}/*.proto)

  foreach(proto_src ${all_protos})
    get_filename_component(proto_name ${proto_src} NAME)

    get_filename_component(module_name ${proto_src} PATH)
    string(TOLOWER ${module_name} module_name)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/${module_name})

    set(output_proto_abs "${CMAKE_CURRENT_BINARY_DIR}/${module_name}/${proto_name}")

    set(proto_src_abs "${root}/${proto_src}")

    add_custom_command(OUTPUT ${output_proto_abs}
                       COMMAND ${CMAKE_SOURCE_DIR}/cmake/fix_pyproto.sh
                       ARGS ${proto_src_abs} ${output_proto_abs}
                       DEPENDS ${proto_src_abs}
                       COMMENT "Adapting ${proto_name} for Python protoc"
                       VERBATIM)
    list(APPEND ${tree} ${output_proto_abs})
  endforeach()

  set_source_files_properties(${${tree}} PROPERTIES GENERATED TRUE)
  set(${tree} ${${tree}} PARENT_SCOPE) 
endfunction()
