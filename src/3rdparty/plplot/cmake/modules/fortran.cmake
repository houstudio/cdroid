# cmake/modules/fortran.cmake
#
# Fortran binding configuration
#
# Copyright (C) 2006  Andrew Ross
# Copyright (C) 2006-2017 Alan W. Irwin
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

# Module for determining Fortran binding configuration options

# Options to enable Fortran binding
if(DEFAULT_NO_BINDINGS)
  option(ENABLE_fortran "Enable fortran binding" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_fortran "Enable fortran binding" ON)
endif(DEFAULT_NO_BINDINGS)

if(ENABLE_fortran AND NOT PLPLOT_Fortran_COMPILER_WORKS)
  workaround_9220(Fortran PLPLOT_Fortran_COMPILER_WORKS)
  if(NOT PLPLOT_Fortran_COMPILER_WORKS)
    message(STATUS "WARNING: no working Fortran compiler so disabling fortran binding and examples.")
    set(ENABLE_fortran OFF CACHE BOOL "Enable fortran binding" FORCE)
  endif(NOT PLPLOT_Fortran_COMPILER_WORKS)
endif(ENABLE_fortran AND NOT PLPLOT_Fortran_COMPILER_WORKS)

if(ENABLE_fortran)
  # Find and check Fortran compiler.
  enable_language(Fortran OPTIONAL)
  if(NOT CMAKE_Fortran_COMPILER_WORKS)
    message(STATUS "WARNING: no working Fortran compiler so disabling fortran binding and examples.")
    set(ENABLE_fortran OFF CACHE BOOL "Enable fortran binding" FORCE)
  endif(NOT CMAKE_Fortran_COMPILER_WORKS)
endif(ENABLE_fortran)

if(ENABLE_fortran)
  # Don't compile Fortran binding if compiler doesn't support it
  if(ENABLE_fortran AND NOT CMAKE_Fortran_COMPILER_SUPPORTS_F90)
    message(STATUS "WARNING: "
    "fortran compiler does not even support Fortran 90 according to CMake.  We need Fortran 2003 so disabling fortran binding"
    )
    set(ENABLE_fortran OFF CACHE BOOL "Enable fortran binding" FORCE)
  endif(ENABLE_fortran AND NOT CMAKE_Fortran_COMPILER_SUPPORTS_F90)
endif(ENABLE_fortran)

if(ENABLE_fortran)
  set(
    FORTRAN_MOD_DIR
    ${LIB_DIR}/fortran/modules/${PACKAGE}
    CACHE PATH "PLplot installation location for fortran modules"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST FORTRAN_MOD_DIR)

  # Check if isnan is available as a fortran function
  include(TestFortranIsnan)

  # Determine which Fortran compiler we have.  We do not need to
  # this for all compilers, just the ones that have a test in
  # bindings/fortran/plstubs.h

  message(STATUS "NOTICE: " "Found: ${CMAKE_Fortran_COMPILER}")
  if(CMAKE_Fortran_COMPILER MATCHES ".*/ifort.*")
    # Intel Visual Fortran
    message(STATUS "NOTICE: " "Found Intel Visual Fortran")
    set(TARGET_FORTRAN "IVF" CACHE STRING "Target Fortran Compiler")
  endif(CMAKE_Fortran_COMPILER MATCHES ".*/ifort.*")

  if(CMAKE_Fortran_COMPILER MATCHES ".*/DF.*")
    # Compaq Visual Fortran
    message(STATUS "NOTICE: " "Found Compaq Visual Fortran")
    set(TARGET_FORTRAN "CVF" CACHE STRING "Target Fortran Compiler")
  endif(CMAKE_Fortran_COMPILER MATCHES ".*/DF.*")

  if(CMAKE_Fortran_COMPILER MATCHES ".*/F90.*" AND WIN32_OR_CYGWIN)
    # Compaq Visual Fortran - alternative name
    message(STATUS "NOTICE: " "Found Compaq Visual Fortran")
    set(TARGET_FORTRAN "CVF" CACHE STRING "Target Fortran Compiler")
  endif(CMAKE_Fortran_COMPILER MATCHES ".*/F90.*" AND WIN32_OR_CYGWIN)

endif(ENABLE_fortran)
