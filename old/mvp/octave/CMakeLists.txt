set(MFILES _mvprefl_findH.m
           _mvprefl_testhelpers.m
           _porthoproj_impl.m
           _porthoproj_impl_ref.m
           gausskernel.m
           geotrans.m
           loadjobfile.cc
           lonlat2normal.m
           lonlatalt2xyz.m
           mvpalgorithm.m
           mvpfootprint.m
           mvpobj.m
           mvpoptimset.m
           mvppatches.m                                                                                                                                                             
           mvpproj.m
           mvprefl.m
           mvprefl_fast.m
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
