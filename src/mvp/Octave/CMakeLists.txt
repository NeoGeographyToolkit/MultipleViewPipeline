mvp_module(NAME Octave
           HDRS Main.h
                ov-mvpobj-ref.h
                oct-mvpobj.h
                OctaveStepper.h
           SRCS Main.cc
                ov-mvpobj-ref.cc)

add_octfile(init_mvp init_mvp.cc)
target_link_libraries(init_mvp ${VW_LIBRARIES}
                               mvpOctave)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
