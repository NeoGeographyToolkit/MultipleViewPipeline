set(MFILES _mvpalgorithm_alt.m
           _mvpalgorithm_orientation.m
           _mvpalgorithm_windows.m
           _porthoproj_impl.m
           _porthoproj_impl_ref.m
           geotrans.m
           loadjobfile.cc
           lonlat2normal.m
           lonlatalt2xyz.m
           mvpalgorithm.m
           mvpfootprint.m
           mvpobj.m
           mvppatches.m
           offsetgeoref.m
           porthoproj.m)

add_octfile(loadjobfile loadjobfile.cc)
target_link_libraries(loadjobfile mvpcommon
                                  ${PROTOBUF_LIBRARIES}
                                  ${VW_LIBRARIES}
                                  ${Boost_LIBRARIES})

install(TARGETS loadjobfile
                DESTINATION ${OCTFILE_INSTALL_DIR})
install(FILES ${MFILES} DESTINATION ${MFILE_INSTALL_DIR})

add_subdirectory(tests)
