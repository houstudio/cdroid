# cmake/modules/ada.cmake
#
# Copyright (C) 2007-2015 Alan W. Irwin
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

# Module for determining Ada binding configuration options

if(DEFAULT_NO_BINDINGS)
  option(ENABLE_ada "Enable Ada binding" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_ada "Enable Ada binding" ON)
endif(DEFAULT_NO_BINDINGS)

if(ENABLE_ada AND NOT PL_DOUBLE)
  message(STATUS "WARNING: "
    "Only single-precision floating point. Disabling Ada binding because of run-time errors in this case.")
  set(ENABLE_ada OFF CACHE BOOL "Enable Ada binding" FORCE)
endif(ENABLE_ada AND NOT PL_DOUBLE)

if(ENABLE_ada AND NOT PLPLOT_Ada_COMPILER_WORKS)
  workaround_9220(Ada PLPLOT_Ada_COMPILER_WORKS)
  if(NOT PLPLOT_Ada_COMPILER_WORKS)
    message(STATUS "WARNING: no working Ada compiler so disabling Ada binding and examples.")
    set(ENABLE_ada OFF CACHE BOOL "Enable Ada binding" FORCE)
  endif(NOT PLPLOT_Ada_COMPILER_WORKS)
endif(ENABLE_ada AND NOT PLPLOT_Ada_COMPILER_WORKS)

if(ENABLE_ada)
  # Find and check Ada compiler
  enable_language(Ada OPTIONAL)
  if(NOT CMAKE_Ada_COMPILER_WORKS)
    message(STATUS "WARNING: no working Ada compiler so disabling Ada binding and examples.")
    set(ENABLE_ada OFF CACHE BOOL "Enable Ada binding" FORCE)
  endif(NOT CMAKE_Ada_COMPILER_WORKS)
endif(ENABLE_ada)

if(ENABLE_ada)
  # The following two default Ada install locations follow the recommendations in
  # <http://www.ada-france.org/debian/debian-ada-policy.html>.
  set(
    ADA_INCLUDE_DIR
    ${CMAKE_INSTALL_DATADIR}/ada/adainclude/plplotada
    CACHE PATH "PLplot install location for (architecture-independent) Ada source files"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST ADA_INCLUDE_DIR)

  set(
    ADA_LIB_DIR
    ${CMAKE_INSTALL_LIBDIR}/ada/adalib/plplotada
    CACHE PATH "PLplot install location for Ada library information files (*.ali)"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST ADA_LIB_DIR)

  # Find the gnat version used in order to search for the right version of libgnat
  execute_process(COMMAND ${CMAKE_Ada_COMPILER} --version OUTPUT_VARIABLE ADA_OUTPUT)
  string(REGEX MATCH "gcc.* [(][^)]*[)] ([0-9]*)([.][0-9]*)[.][0-9]" ADA_OUTPUT_TRIM ${ADA_OUTPUT})
  set(GNATMAJVERSION ${CMAKE_MATCH_1})
  set(GNATVERSION ${CMAKE_MATCH_1}${CMAKE_MATCH_2})
  message(STATUS "gnat version = ${GNATVERSION}")
  find_library(GNAT_LIB NAMES gnat gnat-${GNATVERSION} gnat-${GNATMAJVERSION})
  if(NOT GNAT_LIB)
    message(STATUS "WARNING: "
      "gnat library not found. Disabling Ada binding")
    set(ENABLE_ada OFF CACHE BOOL "Enable Ada binding" FORCE)
  else(NOT GNAT_LIB)
    message(STATUS "FOUND gnat library ${GNAT_LIB}")
  endif(NOT GNAT_LIB)
endif(ENABLE_ada)
