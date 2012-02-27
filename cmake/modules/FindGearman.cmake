if (NOT "$ENV{GEAR<AN_ROOT}" STREQUAL "")
  set(GEARMAN_ROOT $ENV{GEARMAN_ROOT})
endif()

# TODO: if ${GEARMAN_ROOT} is empty, the hint is /include rather than nothing
# it would be better to do _search_paths like FindBoost does.
find_path(GEARMAN_INCLUDE_DIR libgearman/gearman.h HINTS ${GEARMAN_ROOT}/include)

find_library(GEARMAN_LIBRARY NAMES gearman HINTS ${GEARMAN_ROOT}/lib)

set(GEARMAN_LIBRARIES ${GEARMAN_LIBRARY})
set(GEARMAN_INCLUDE_DIRS ${GEARMAN_INCLUDE_DIR})

find_package_handle_standard_args(Gearman REQUIRED_VARS GEARMAN_LIBRARY GEARMAN_INCLUDE_DIR)
