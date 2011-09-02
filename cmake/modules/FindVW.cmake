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

macro(find_vw_components _component_list _components_req)
  foreach(comp ${${_component_list}})
    string(TOUPPER ${comp} ucomp)
    find_library(VW_${ucomp}_LIBRARY
                 NAMES vw${comp}
                 HINTS ${VW_ROOT}/lib)
    mark_as_advanced(VW_${comp}_LIBRARY)
    if (VW_${ucomp}_LIBRARY)
      set(VW_${ucomp}_FOUND true)
    else()
      if (${_components_req} STREQUAL "REQUIRED")
        message(SEND_ERROR "Unable to find required VW component ${comp}")
      endif()
    endif()
  endforeach()
endmacro()

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

find_path(VW_INCLUDE_DIR
          NAMES vw/config.h
          HINTS ${VW_ROOT}/include)
mark_as_advanced(VW_INCLUDE_DIR)

if (NOT VW_INCLUDE_DIR)
  vw_not_found_err("Unable to find VW include dir")
  return()
endif()

file(READ ${VW_INCLUDE_DIR}/vw/config.h _vw_config_contents)
string(REGEX REPLACE ".*#define VW_BOOST_VERSION ([0-9]+).*" "\\1" VW_BOOST_VERSION "${_vw_config_contents}")

if (VW_DEBUG)
  message(STATUS "VW uses boost version: ${VW_BOOST_VERSION}")
endif()

find_package(Boost QUIET)

if (NOT Boost_FOUND)
  vw_not_found_err("Prerequisite Boost not found")
  return()
endif()


if (NOT ${Boost_VERSION} STREQUAL ${VW_BOOST_VERSION})
  vw_not_found_err("Boost version mismatch. VW was compiled with boost version ${VW_BOOST_VERSION}, and the detected Boost was ${Boost_VERSION}. Make sure to export BOOST_ROOT=/path/to/boost, then clear cmake's cache and try again.")
  return()
endif()

# Find VW
find_library(VW_LIBRARY
             NAMES vw
             HINTS ${VW_ROOT}/lib)
if (VW_LIBRARY)
  set(VW_LIBRARY_FOUND true)
else()
  vw_not_found_err("Unable to find libvw")
endif()


# Find VW components
set(_req_component_list Core FileIO Image)
set(_opt_component_list BundleAdjustment 
                        Camera 
                        Cartography
                        Geometry
                        GPU
                        HDR
                        InterestPoint
                        Math
                        Mosaic
                        Photometry
                        Plate
                        Python
                        Stereo
                        Tools)

if (VW_FIND_REQUIRED)
  foreach (comp ${VW_FIND_COMPONENTS})
    list(APPEND _req_component_list ${comp})
    list(REMOVE_ITEM _opt_component_list ${comp})
  endforeach()
  list(REMOVE_DUPLICATES _req_component_list)
endif()

find_vw_components(_req_component_list REQUIRED)
find_vw_components(_opt_component_list OPTIONAL)

set(VW_INCLUDE_DIRS ${VW_INCLUDE_DIR} ${Boost_INCLUDE_DIRS})
set(VW_LIBRARIES ${VW_LIBRARY})
set(VW_FOUND true)
