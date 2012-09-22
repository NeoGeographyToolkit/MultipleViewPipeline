mvp_module(NAME Octave
           HDRS Main.h
                ov-mvpobj-ref.h
                oct-mvpobj.h
           SRCS Main.cc)

add_octfile(init_mvp init_mvp.cc)
target_link_libraries(init_mvp ${VW_LIBRARIES})

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
