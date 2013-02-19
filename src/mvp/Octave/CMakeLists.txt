mvp_module(NAME Octave
           DEPS mvpCore
                ${OCTAVE_LIBRARIES}
           HDRS Main.h
                oct-mvpclass.h
                ov-mvpclass-ref.h
                MvpWrapper.h
           SRCS Main.cc
                MvpWrapper.cc
                Conversions.cc
                ov-mvpclass-ref.cc
                wraps/Dummy.cc)

add_octfile(init_mvp init_mvp.cc)
target_link_libraries(init_mvp mvpOctave mvpAlgorithm)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
