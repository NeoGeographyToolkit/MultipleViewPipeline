mvp_module(NAME Octave
           DEPS mvpCore
                ${OCTAVE_LIBRARIES}
           HDRS Main.h
                ov-mvpobj-ref.h
                oct-mvpobj.h
           SRCS Main.cc
                ov-mvpobj-ref.cc)

add_octfile(init_mvp init_mvp.cc)
target_link_libraries(init_mvp mvpOctave)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
