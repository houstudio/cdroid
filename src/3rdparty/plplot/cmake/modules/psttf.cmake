# cmake/modules/psttf.cmake
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

# Configuration for the psttf device driver (supporting the psttf and
# psttfc devices).
#
# The following variables are set / modified
#
# PLD_psttf               - ON means the psttf device is enabled.
# PLD_psttfc              - ON means the psttfc device is enabled.
# psttf_COMPILE_FLAGS  	  - blank-separated COMPILE_FLAGS required to
#			    compile psttf device driver.
# psttf_LINK_FLAGS     	  - list of LINK_FLAGS for dynamic psttf device driver.
# psttf_RPATH	       	  - RPATH directory list for psttf device driver.
# DRIVERS_LINK_FLAGS  	  - list of device LINK_FLAGS and TARGETS for case
# 			    when ENABLE_DYNDRIVERS OFF.

# Include file searches use FindPath. To add extra search directories
# set the environment variable CMAKE_INCLUDE_PATH.
# Library searches use FindLibrary. To add extra search directories
# set the environment variable CMAKE_LIBRARY_PATH.
# See cmake documentation for further details.

if(PLD_psttf OR PLD_psttfc)
  if(NOT CMAKE_CXX_COMPILER_WORKS)
    message(STATUS "WARNING: no working C++ compiler so "
      "setting PLD_psttf and PLD_psttfc to OFF."
      )
    set(PLD_psttf OFF CACHE BOOL "Enable psttf device" FORCE)
    set(PLD_psttfc OFF CACHE BOOL "Enable psttfc device" FORCE)
  endif(NOT CMAKE_CXX_COMPILER_WORKS)
endif(PLD_psttf OR PLD_psttfc)

# Look for psttf headers and libraries with pkg-config
if(PLD_psttf OR PLD_psttfc)
  if(NOT PKG_CONFIG_EXECUTABLE)
    message(STATUS
    "WARNING: pkg-config not found so setting PLD_psttf and PLD_psttfc to OFF."
    )
    set(PLD_psttf OFF CACHE BOOL "Enable psttf device" FORCE)
    set(PLD_psttfc OFF CACHE BOOL "Enable psttfc device" FORCE)
  endif(NOT PKG_CONFIG_EXECUTABLE)
endif(PLD_psttf OR PLD_psttfc)

if(PLD_psttf OR PLD_psttfc)
  pkg_check_pkgconfig("lasi;pango;pangoft2" includedir libdir linkflags cflags version _PSTTF)
  if(linkflags)
    #blank-separated required.
    string(REGEX REPLACE ";" " " psttf_COMPILE_FLAGS "${cflags}")
    set(psttf_LINK_FLAGS ${linkflags})
    set(psttf_RPATH ${libdir})
    filter_rpath(psttf_RPATH)
    list(APPEND DRIVERS_LINK_FLAGS ${psttf_LINK_FLAGS})
  else(linkflags)
    message("includedir = ${includedir}")
    message("libdir = ${libdir}")
    message("linkflags = ${linkflags}")
    message("cflags = ${cflags}")
    message(STATUS
       "WARNING: pango, pangoft2, or lasi not found with pkg-config.\n"
    "   Setting PLD_psttf and PLD_psttfc to OFF.  Please install all of these packages\n"
    "   and/or set the environment variable PKG_CONFIG_PATH appropriately."
    )
    set(PLD_psttf OFF CACHE BOOL "Enable psttf device" FORCE)
    set(PLD_psttfc OFF CACHE BOOL "Enable psttfc device" FORCE)
  endif(linkflags)
endif(PLD_psttf OR PLD_psttfc)
# Test for correct version of liblasi by looking
# for API that was added for 1.0.5 which is required by PLplot.???
