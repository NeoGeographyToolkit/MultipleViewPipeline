add_subdirectory(Core)

set(MVP_HDRS ${CMAKE_CURRENT_BINARY_DIR}/Config.h)

configure_file(Config.h.in Config.h)

install(FILES ${MVP_HDRS} DESTINATION include/mvp)
