# cmake/modules/d.cmake
#
# D binding configuration
#
# Copyright (C) 2008  Werner Smekal
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

# Module for determining D binding configuration options

# Options to enable D binding
if(DEFAULT_NO_BINDINGS)
  option(ENABLE_d "Enable D binding" OFF)
elseif(WIN32_AND_NOT_CYGWIN)
  option(ENABLE_d "Enable D binding" OFF)
  if(ENABLE_d)
    message(STATUS "WARNING: setting the -DENABLE_d=ON option is considered to be experimental on Windows because it has been found to interfere with wxwidgets linking for unknown reasons")
  endif(ENABLE_d)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_d "Enable D binding" ON)
endif(DEFAULT_NO_BINDINGS)

if(ENABLE_d AND NOT PL_DOUBLE)
  message(STATUS "WARNING: "
    "Only single-precision floating point. Disabling D binding because of run-time errors in this case.")
  set(ENABLE_d OFF CACHE BOOL "Enable D binding" FORCE)
endif(ENABLE_d AND NOT PL_DOUBLE)

if(ENABLE_d AND NOT PLPLOT_D_COMPILER_WORKS)
  workaround_9220(D PLPLOT_D_COMPILER_WORKS)
  if(NOT PLPLOT_D_COMPILER_WORKS)
    message(STATUS "WARNING: no working D compiler so disabling D binding and examples.")
    set(ENABLE_d OFF CACHE BOOL "Enable D binding" FORCE)
  endif(NOT PLPLOT_D_COMPILER_WORKS)
endif(ENABLE_d AND NOT PLPLOT_D_COMPILER_WORKS)

if(ENABLE_d)
  # Find and check D compiler
  set(CMAKE_D_USE_PHOBOS ON)
  enable_language(D OPTIONAL)
  if(NOT CMAKE_D_COMPILER_WORKS)
    message(STATUS "WARNING: no working D compiler so disabling D binding and examples.")
    set(ENABLE_d OFF CACHE BOOL "Enable D binding" FORCE)
  endif(NOT CMAKE_D_COMPILER_WORKS)
endif(ENABLE_d)

if(ENABLE_d)
  # Set this option to OFF by default since the STATIC library is
  # the only one that seems to work for now on Debian Squeeze and
  # potentially other Linux systems.
  option(plplotdmd_SHARED "Treat libplplotdmd as a shared library" OFF)
endif(ENABLE_d)

