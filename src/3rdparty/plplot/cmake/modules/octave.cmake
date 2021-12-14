# cmake/modules/octave.cmake
#
# Copyright (C) 2006-2018 Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Module for determining Octave binding configuration options

# Options to enable Octave binding
if(DEFAULT_NO_BINDINGS)
  option(ENABLE_octave "Enable Octave binding" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_octave "Enable Octave binding" ON)
endif(DEFAULT_NO_BINDINGS)

if(ENABLE_octave AND NOT CMAKE_CXX_COMPILER_WORKS)
  message(STATUS "WARNING: "
    "Octave requires working C++ compiler to build.  Disabling Octave binding")
  set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
endif(ENABLE_octave AND NOT CMAKE_CXX_COMPILER_WORKS)

if(ENABLE_octave AND NOT SWIG_FOUND)
  message(STATUS "WARNING: "
    "swig not found. Disabling Octave binding")
  set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
endif(ENABLE_octave AND NOT SWIG_FOUND)

if(ENABLE_octave AND NOT BUILD_SHARED_LIBS)
  message(STATUS "WARNING: "
    "Octave requires shared libraries. Disabling Octave binding")
  set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
endif(ENABLE_octave AND NOT BUILD_SHARED_LIBS)

if(ENABLE_octave AND NOT PL_DOUBLE)
  message(STATUS "WARNING: "
    "Only single-precision floating point. Disabling Octave binding because of build errors in this case.")
  set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
endif(ENABLE_octave AND NOT PL_DOUBLE)

if(ENABLE_octave AND NOT PERL_FOUND)
  message(STATUS "WARNING: "
    "The Octave build requires perl. Disabling Octave binding")
  set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
endif(ENABLE_octave AND NOT PERL_FOUND)

if(ENABLE_octave)
  find_program(OCTAVE octave)
  if(OCTAVE)
    message(STATUS "OCTAVE = ${OCTAVE}")
  else(OCTAVE)
    message(STATUS "WARNING: "
    "The octave command not found. Disabling Octave binding")
    set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
  endif(OCTAVE)
endif(ENABLE_octave)

if(ENABLE_octave)
  find_program(MKOCTFILE mkoctfile)
  if(MKOCTFILE)
    message(STATUS "MKOCTFILE = ${MKOCTFILE}")
  else(MKOCTFILE)
    message(STATUS "WARNING: "
    "The mkoctfile command not found. Disabling Octave binding")
    set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
  endif(MKOCTFILE)
endif(ENABLE_octave)

if(ENABLE_octave)
  find_program(OCTAVE_CONFIG octave-config)
  if(OCTAVE_CONFIG)
    message(STATUS "OCTAVE_CONFIG = ${OCTAVE_CONFIG}")
  else(OCTAVE_CONFIG)
    message(STATUS "WARNING: "
    "The octave-config command not found. Disabling Octave binding")
    set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
  endif(OCTAVE_CONFIG)
endif(ENABLE_octave)

if(ENABLE_octave)
  #OCTAVE_VERSION is the (dotted triplet) octave version.
  execute_process(
    COMMAND ${OCTAVE_CONFIG} -p VERSION
    OUTPUT_VARIABLE OCTAVE_VERSION
    ERROR_VARIABLE OCTAVE_ERROR
    RESULT_VARIABLE return_code
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  if(return_code)
    message(STATUS "OCTAVE_ERROR = ${OCTAVE_ERROR}")
    message(STATUS "WARNING: "
    "${OCTAVE_CONFIG} -p VERSION generates an error (non-zero return code).  Disabling Octave binding")
    set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
  endif(return_code)
endif(ENABLE_octave)

if(ENABLE_octave)
  message(STATUS "OCTAVE_VERSION = ${OCTAVE_VERSION}")
  if(${OCTAVE_VERSION} VERSION_LESS "3.8.0")
    message(STATUS "WARNING: "
    "PLplot requires Octave version 3.8.0 or greater. Disabling Octave binding")
    set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
  endif(${OCTAVE_VERSION} VERSION_LESS "3.8.0")
endif(ENABLE_octave)

if(ENABLE_octave)
  # The following if block is required to keep from repeated pre-pending
  # of OCTAVE_INCLUDE_PATH_TRIMMED onto OCTAVE_INCLUDE_PATH which would
  # create 2^n components to OCTAVE_INCLUDE_PATH for n cmake calls.
  # Note, all externally used results (OCTAVE_INCLUDE_PATH, OCTAVE_LIBRARIES,
  # OCTINTERP_LIBRARIES, and ENABLE_octave) are cached so skipping everything
  # if OCTAVE_INCLUDE_PATH is defined from the previous cmake run should be
  # fine.
  if(NOT DEFINED OCTAVE_INCLUDE_PATH)
    execute_process(
    COMMAND ${OCTAVE_CONFIG} -p OCTINCLUDEDIR
    OUTPUT_VARIABLE OCTAVE_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    find_path(
    OCTAVE_INCLUDE_PATH
    oct.h
    HINTS ${OCTAVE_INCLUDE_DIR}
    PATH_SUFFIXES octave
    )

    execute_process(
    COMMAND ${OCTAVE_CONFIG} -p OCTLIBDIR
    OUTPUT_VARIABLE OCTAVE_LIB_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    find_library(
    OCTAVE_LIBRARIES
    octave
    HINTS ${OCTAVE_LIB_DIR}
    )

    find_library(
    OCTINTERP_LIBRARIES
    octinterp
    HINTS ${OCTAVE_LIB_DIR}
    )

    if(OCTAVE_INCLUDE_PATH AND OCTAVE_LIBRARIES AND OCTINTERP_LIBRARIES)
      # If first octave include path has trailing /octave, then must have
      # second include path without that trailing /octave.
      string(REGEX REPLACE "/octave$" ""
      OCTAVE_INCLUDE_PATH_TRIMMED
      ${OCTAVE_INCLUDE_PATH}
      )
      if(NOT OCTAVE_INCLUDE_PATH_TRIMMED STREQUAL "${OCTAVE_INCLUDE_PATH}")
        set(OCTAVE_INCLUDE_PATH
	${OCTAVE_INCLUDE_PATH_TRIMMED} ${OCTAVE_INCLUDE_PATH}
	CACHE INTERNAL ""
	)
      endif(NOT OCTAVE_INCLUDE_PATH_TRIMMED STREQUAL "${OCTAVE_INCLUDE_PATH}")
      # Octave has a huge number of dependencies and therefore an
      # impossible-to-untangle set of header #includes that depend on
      # other packages headers.  And there is no information from
      # mkoctfile or the octave_config command about where
      # those header directories are located.  But from experiments
      # with both the Linux and Windows binary versions of octave, it
      # appears that hdf5.h is one external header that is necessary,
      # and it is never part of the octave-${OCTAVE_VERSION}/octave
      # hierarchy.
      # N.B. Debian jessie octave-dev package depends on
      # hdf5-dev which stores hdf5.h in /usr/include/hdf5/serial
      find_path(
	OCTAVE_INCLUDE_PATH_EXTERNAL
	hdf5.h PATH_SUFFIXES hdf5/serial
	)
      if(OCTAVE_INCLUDE_PATH_EXTERNAL)
        set(OCTAVE_INCLUDE_PATH
	${OCTAVE_INCLUDE_PATH_EXTERNAL} ${OCTAVE_INCLUDE_PATH}
	CACHE INTERNAL ""
	)
      else(OCTAVE_INCLUDE_PATH_EXTERNAL)
	message(STATUS "WARNING: "
	  "Required external Octave header, hdf5.h, that is typically part of the libhdf5-dev package, has not found. Disabling Octave binding")
	set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
      endif(OCTAVE_INCLUDE_PATH_EXTERNAL)
    else(OCTAVE_INCLUDE_PATH AND OCTAVE_LIBRARIES AND OCTINTERP_LIBRARIES)
      message(STATUS "WARNING: "
      "The Octave headers and/or library not found. Disabling Octave binding")
      set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
    endif(OCTAVE_INCLUDE_PATH AND OCTAVE_LIBRARIES AND OCTINTERP_LIBRARIES)
  endif(NOT DEFINED OCTAVE_INCLUDE_PATH)
  message(STATUS "OCTAVE_LIBRARIES = ${OCTAVE_LIBRARIES}")
  message(STATUS "OCTINTERP_LIBRARIES = ${OCTINTERP_LIBRARIES}")
  message(STATUS "OCTAVE_INCLUDE_PATH = ${OCTAVE_INCLUDE_PATH}")
endif(ENABLE_octave)

if(ENABLE_octave)

  # Determine OCTAVE_PREFIX which is the prefix where octave was installed.

  if(NOT OCTAVE_PREFIX)
    execute_process(
      COMMAND ${OCTAVE_CONFIG} -p PREFIX
      OUTPUT_VARIABLE OCTAVE_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  endif(NOT OCTAVE_PREFIX)

  if(NOT OCTAVE_PREFIX)
    # PREFIX no longer available for octave-4.4 so try alternative of OCTAVE_HOME which is
    # available for that version of Octave.
    execute_process(
      COMMAND ${OCTAVE_CONFIG} -p OCTAVE_HOME
      OUTPUT_VARIABLE OCTAVE_PREFIX
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
  endif(NOT OCTAVE_PREFIX)

  #message(STATUS "OCTAVE_PREFIX = ${OCTAVE_PREFIX}")
  file(TO_CMAKE_PATH "${OCTAVE_PREFIX}" OCTAVE_PREFIX)
  #message(STATUS "(CMake) OCTAVE_PREFIX = ${OCTAVE_PREFIX}")

  if(NOT OCTAVE_PREFIX)
    message(STATUS "OCTAVE_PREFIX = ${OCTAVE_PREFIX}")
    message(STATUS "WARNING: Valid value of OCTAVE_PREFIX could not be determined.  Disabling Octave binding")
    set(ENABLE_octave OFF CACHE BOOL "Enable Octave binding" FORCE)
  endif(NOT OCTAVE_PREFIX)
endif(ENABLE_octave)

if(ENABLE_octave)

  # Determine _octave_m_dir, the uncached version of OCTAVE_M_DIR
  execute_process(
    COMMAND ${OCTAVE_CONFIG} -p LOCALFCNFILEDIR
    OUTPUT_VARIABLE _octave_m_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  #message(STATUS "_octave_m_dir = ${_octave_m_dir}")
  file(TO_CMAKE_PATH "${_octave_m_dir}" _octave_m_dir)
  #message(STATUS "(CMake) _octave_m_dir = ${_octave_m_dir}")

  # Replace the OCTAVE_PREFIX with the PLplot prefix in _octave_m_dir
  # and append "/PLplot"
  string(REPLACE
    "${OCTAVE_PREFIX}"
    "${CMAKE_INSTALL_PREFIX}"
    _octave_m_dir
    "${_octave_m_dir}/PLplot"
    )
  #message(STATUS "Transformed _octave_m_dir = ${_octave_m_dir}")

  # Determine _octave_oct_dir, the uncached version of OCTAVE_OCT_DIR
  execute_process(
    COMMAND ${OCTAVE_CONFIG} -p LOCALOCTFILEDIR
    OUTPUT_VARIABLE _octave_oct_dir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

    #message(STATUS "_octave_oct_dir = ${_octave_oct_dir}")
    file(TO_CMAKE_PATH "${_octave_oct_dir}" _octave_oct_dir)
    #message(STATUS "(CMake) _octave_oct_dir = ${_octave_oct_dir}")

  # Specify _octave_oct_dir if prefixes not the same.
  if(NOT CMAKE_INSTALL_PREFIX STREQUAL "${OCTAVE_PREFIX}")
    set(_octave_oct_dir ${CMAKE_INSTALL_LIBDIR}/octave)
    #message(STATUS "Specified _octave_oct_dir = ${_octave_oct_dir}")
  endif(NOT CMAKE_INSTALL_PREFIX STREQUAL "${OCTAVE_PREFIX}")

  # Results for cached installation directory variables.
  set(
    PLPLOT_OCTAVE_DIR
    "${CMAKE_INSTALL_DATADIR}/plplot_octave"
    CACHE PATH "PLplot install location for (*.m) files needed for the PLplot octave binding"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST PLPLOT_OCTAVE_DIR)

  set(
    OCTAVE_M_DIR
    "${_octave_m_dir}"
    CACHE PATH "PLplot install location for (*.m) files that provide useful additional functionality"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST OCTAVE_M_DIR)

  set(
    OCTAVE_OCT_DIR
    "${_octave_oct_dir}"
    CACHE PATH "PLplot install location for Octave binding shared object (plplot_octave.oct) file"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST OCTAVE_OCT_DIR)

endif(ENABLE_octave)
