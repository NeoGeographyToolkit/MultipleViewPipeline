# octave-config is used to find and detect all other programs / flags

# Note that ALL_CXXFLAGS contains INCFLAGS, CXXFLAGS and XTRA_CXXFLAGS
#           ALL_LDFLAGS contains LD_STATIC_FLAG, CPICFLAG, and LDFLAGS

include(FindPackageHandleStandardArgs)
include(FindPackageMessage)

# OCTAVE_CONFIG_EXECUTABLE
find_program(OCTAVE_CONFIG_EXECUTABLE NAME octave-config HINTS ${OCTAVE_CONFIG_EXECUTABLE})
mark_as_advanced(OCTAVE_CONFIG_EXECUTABLE)

# OCTAVE_VERSION
execute_process(
  COMMAND ${OCTAVE_CONFIG_EXECUTABLE} -p VERSION
  OUTPUT_VARIABLE _octave_version
  RESULT_VARIABLE _octave_config_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_version "${_octave_version}")
set(OCTAVE_VERSION "${_octave_version}")

# OCTAVE_BINDIR
execute_process(
  COMMAND ${OCTAVE_CONFIG_EXECUTABLE} -p BINDIR
  OUTPUT_VARIABLE _octave_bindir
  RESULT_VARIABLE _octave_config_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_bindir "${_octave_bindir}")
set(OCTAVE_BINDIR "${_octave_bindir}")

# OCTAVE_OCTFILEDIR
execute_process(
  COMMAND ${OCTAVE_CONFIG_EXECUTABLE} -p LOCALVEROCTFILEDIR
  OUTPUT_VARIABLE _octave_config_localveroctfiledir
  RESULT_VARIABLE _octave_config_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_config_localveroctfiledir "${_octave_config_localveroctfiledir}")
set(OCTAVE_OCTFILEDIR "${_octave_config_localveroctfiledir}")

# MKOCTFILE_EXECUTABLE
set(MKOCTFILE_EXECUTABLE "${OCTAVE_BINDIR}/mkoctfile")

# OCTAVE_EXECUTABLE
set(OCTAVE_EXECUTABLE "${OCTAVE_BINDIR}/octave")

# OCTAVE_COMPILE_FLAGS
execute_process(
  COMMAND ${MKOCTFILE_EXECUTABLE} -p CXXFLAGS
  OUTPUT_VARIABLE _octave_cxxflags
  RESULT_VARIABLE _mkoctfile_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_cxxflags "${_octave_cxxflags}")

execute_process(
  COMMAND ${MKOCTFILE_EXECUTABLE} -p XTRA_CXXFLAGS
  OUTPUT_VARIABLE _octave_xtra_cxxflags
  RESULT_VARIABLE _mkoctfile_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_xtra_cxxflags "${_octave_xtra_cxxflags}")

string(REGEX REPLACE " +$" "" _octave_cxxflags "${_octave_cxxflags} ${_octave_xtra_cxxflags}")
set(OCTAVE_COMPILE_FLAGS "${_octave_cxxflags}")

# OCTAVE_INCLUDE_DIRS
execute_process(
  COMMAND ${MKOCTFILE_EXECUTABLE} -p INCFLAGS
  OUTPUT_VARIABLE _octave_incflags
  RESULT_VARIABLE _mkoctfile_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_incflags "${_octave_incflags}")
string(REGEX REPLACE " +-I" " " _octave_incflags " ${_octave_incflags}")
string(REGEX REPLACE "^ " "" _octave_incflags "${_octave_incflags}")
separate_arguments(_octave_incflags)
set(OCTAVE_INCLUDE_DIRS "${_octave_incflags}")

# OCTAVE_LIBRARIES
execute_process(
  COMMAND ${MKOCTFILE_EXECUTABLE} -p LIBS
  OUTPUT_VARIABLE _octave_libs
  RESULT_VARIABLE _mkoctfile_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_libs "${_octave_libs}")

execute_process(
  COMMAND ${MKOCTFILE_EXECUTABLE} -p OCTAVE_LIBS
  OUTPUT_VARIABLE _octave_octave_libs
  RESULT_VARIABLE _mkoctfile_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_octave_libs "${_octave_octave_libs}")

string(REGEX REPLACE " +-l" " " _octave_libs " ${_octave_libs} ${_octave_octave_libs}")
string(REGEX REPLACE "^ " "" _octave_libs "${_octave_libs}")
separate_arguments(_octave_libs)
set(OCTAVE_LIBRARIES "${_octave_libs}")

# OCTAVE_LINK_FLAGS
execute_process(
  COMMAND ${MKOCTFILE_EXECUTABLE} -p ALL_LDFLAGS
  OUTPUT_VARIABLE _octave_all_ldflags
  RESULT_VARIABLE _mkoctfile_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_all_ldflags "${_octave_all_ldflags}")
separate_arguments(_octave_all_ldflags)
set(OCTAVE_LINK_FLAGS "${_octave_all_ldflags}")

# OCTAVE_LINK_DIRECTORIES
execute_process(
  COMMAND ${MKOCTFILE_EXECUTABLE} -p LFLAGS
  OUTPUT_VARIABLE _octave_lflags
  RESULT_VARIABLE _mkoctfile_failed)
string(REGEX REPLACE "[\r\n]$" "" _octave_lflags "${_octave_lflags}")
string(REGEX REPLACE " +-L" " " _octave_lflags " ${_octave_lflags}")
string(REGEX REPLACE "^ " "" _octave_lflags "${_octave_lflags}")
separate_arguments(_octave_lflags)
set(OCTAVE_LINK_DIRECTORIES "${_octave_lflags}")

# Wrap up
find_package_handle_standard_args(Octave REQUIRED_VARS OCTAVE_EXECUTABLE OCTAVE_CONFIG_EXECUTABLE MKOCTFILE_EXECUTABLE)

if (OCTAVE_FOUND)
  find_package_message(Octave "Found Octave version: ${OCTAVE_VERSION}" "[${OCTAVE_CONFIG_EXECUTABLE}]")
endif()
