include_directories(${MVP_DIR}/src)

add_octfile(_init_mvp _init_mvp.cc ${MVP_DIR}/src/mvp/Octave/ov-mvpobj-ref.cc)
add_octfile(_back_project _back_project.cc)
add_octfile(_do_homog _do_homog.cc)

add_subdirectory(tests)

include_directories(${CMAKE_SOURCE_DIR}/thirdparty/gtest)
