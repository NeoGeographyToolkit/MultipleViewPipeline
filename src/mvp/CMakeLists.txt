# Find VW packages
find_package(VW REQUIRED COMPONENTS Cartography Camera Plate)
include_directories(${VW_INCLUDE_DIRS})

find_package(Protobuf REQUIRED)
include_directories(${PROTOBUF_INCLUDE_DIRS})


include_directories(${CMAKE_CURRENT_BINARY_DIR})
PROTOBUF_GENERATE_CPP(PROTO_SRCS PROTO_HDRS MVPJobRequest.proto
                                            OrbitalImageDesc.proto)


add_executable(mvp_start mvp_start.cc ${PROTO_SRCS})
target_link_libraries(mvp_start ${PROTOBUF_LIBRARIES})

#if (BUILD_TESTS)
#  add_subdirectory(tests)
#endif()
