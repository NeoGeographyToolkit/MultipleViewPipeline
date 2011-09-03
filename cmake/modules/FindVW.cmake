#
# VW_FOUND
# VW_LIBRARIES
# VW_INCLUDE_DIRS
# VW_BOOST_VERSION
# 
# 
# VW_${COMPONENT}_FOUND
# VW_${COMPONENT}_LIBRARY
#

macro(vw_not_found_err _msg)
  if (VW_FIND_REQUIRED)
    message(SEND_ERROR ${_msg})
  else()
    if (NOT VW_FIND_QUIETLY)
      message(STATUS ${_msg})
    endif()
  endif()
endmacro()

set(VW_ROOT $ENV{VW_ROOT})
set(VW_FOUND false)

if (NOT VW_LIBRARY)
  find_path(VW_INCLUDE_DIR
            NAMES vw/config.h
            HINTS ${VW_ROOT}/include)
  mark_as_advanced(VW_INCLUDE_DIR)

  if (NOT VW_INCLUDE_DIR)
    vw_not_found_err("Unable to find VW include dir")
    return()
  endif()

  if (NOT VW_FIND_QUIETLY)
    message(STATUS "Found VW include dir: ${VW_INCLUDE_DIR}")
  endif()

  # Find VW library
  find_library(VW_LIBRARY
               NAMES vw
               HINTS ${VW_ROOT}/lib)
  if (VW_LIBRARY)
    set(VW_LIBRARY_FOUND true)
  else()
    vw_not_found_err("Unable to find libvw")
  endif()
endif()

set(VW_LIBRARIES ${VW_LIBRARY})

# Find VW components
foreach(comp ${VW_FIND_COMPONENTS})
  string(TOUPPER ${comp} ucomp)
  find_library(VW_${ucomp}_LIBRARY
               NAMES vw${comp}
               HINTS ${VW_ROOT}/lib)
  mark_as_advanced(VW_${ucomp}_LIBRARY)
  if (VW_${ucomp}_LIBRARY)
    set(VW_${ucomp}_FOUND true)
    set(VW_LIBRARIES ${VW_LIBRARIES} ${VW_${ucomp}_LIBRARY})
  else()
    if (VW_FIND_REQUIRED)
      message(SEND_ERROR "Unable to find required VW component ${comp}")
    endif()
  endif()
endforeach()

# Find the prerequisite libraries in boost, also check to insure versions match
file(READ ${VW_INCLUDE_DIR}/vw/config.h _vw_config_contents)
string(REGEX REPLACE ".*#define VW_BOOST_VERSION ([0-9]+).*" "\\1" VW_BOOST_VERSION "${_vw_config_contents}")

if (VW_DEBUG)
  message(STATUS "VW uses boost version: ${VW_BOOST_VERSION}")
endif()

find_package(Boost REQUIRED COMPONENTS filesystem thread system)

if (NOT ${Boost_VERSION} STREQUAL ${VW_BOOST_VERSION})
  vw_not_found_err("Boost version mismatch. VW was compiled with boost version ${VW_BOOST_VERSION}, and the detected Boost was ${Boost_VERSION}. Make sure to export BOOST_ROOT=/path/to/boost, then clear cmake's cache and try again.")
  return()
endif()

# Final setting of vars
set(VW_INCLUDE_DIRS ${VW_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})

# For some reason, on OSX you have to link explicitly to the boost libraries
# when also linking to VW, so we add them to the list.
set(VW_LIBRARIES ${VW_LIBRARIES} ${Boost_LIBRARIES})

set(VW_FOUND true)
