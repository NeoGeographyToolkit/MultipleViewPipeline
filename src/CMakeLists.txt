cmake_minimum_required (VERSION 2.6)
project (AMVP)

include_directories(~/projects/VisionWorkbench/build/include)
link_directories(~/projects/VisionWorkbench/build/lib)

add_executable(gen_synth_scene gen_synth_scene.cc)
target_link_libraries(gen_synth_scene vw vwCartography vwCamera)
