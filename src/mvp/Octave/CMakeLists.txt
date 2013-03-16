mvp_module(NAME Octave
           DEPS mvpCore
                mvpAlgorithm
                ${OCTAVE_LIBRARIES}
           HDRS Main.h
                oct-mvpclass.h
                ov-mvpclass-ref.h
                MvpWrapper.h
                OctaveWrapper.h
           SRCS Main.cc
                MvpWrapper.cc
                OctaveWrapper.cc
                ov-mvpclass-ref.cc
                wraps/Dummy.cc
                wraps/Datum.cc
                wraps/GeoReference.cc
           )

add_octfile(init_mvp init_mvp.cc)
target_link_libraries(init_mvp mvpOctave mvpAlgorithm)

if (BUILD_TESTS)
  add_subdirectory(tests)
endif()
