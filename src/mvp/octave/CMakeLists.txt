set(MFILES mvpalgorithm.m
           lonlat2normal.m
           lonlatalt2xyz.m)

add_octfile(loadjobfile loadjobfile.cc)
target_link_libraries(loadjobfile mvpcommon
                                  ${PROTOBUF_LIBRARIES}
                                  ${VW_LIBRARIES}
                                  ${Boost_LIBRARIES})

install(TARGETS loadjobfile
                DESTINATION ${OCTFILE_INSTALL_DIR})
install(FILES ${MFILES} DESTINATION ${MFILE_INSTALL_DIR})

add_subdirectory(tests)
