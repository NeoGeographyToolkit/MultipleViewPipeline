add_subdirectory(Core)
add_subdirectory(Pipeline)

configure_file(Config.h.in Config.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Config.h DESTINATION include/mvp)
