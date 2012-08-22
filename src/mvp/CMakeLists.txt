add_subdirectory(Core)
add_subdirectory(Pipeline)
add_subdirectory(Frontend)
add_subdirectory(OrbitalImage)

configure_file(Config.h.in Config.h)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Config.h DESTINATION include/mvp)
