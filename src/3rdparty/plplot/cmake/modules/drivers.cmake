# cmake/modules/drivers.cmake
#
# Start driver initializations
#
# Copyright (C) 2006-2015  Alan W. Irwin
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

# Module for configuring all device-related variables.

# Results are contained in the following variables:
# ENABLE_DYNDRIVERS (ON or OFF): whether to dynamically load device drivers.
# PLD_devicename (ON or OFF): whether each PLplot-related device is enabled
#   or not.  devicename is png, jpeg, etc.
# devicename_COMPILE_FLAGS: compile (e.g., -I) options for each individual
#   device.
# devicename_LINK_FLAGS: link options (e.g., -L  and -l) for each
#   individual device.
# DRIVERS_LINK_FLAGS: concatenated link options for all devices.
# DEVICES_LIST: list of devices (e.g. png, jpeg),where PLD_devicename is ON
# DRIVERS_LIST: list of device drivers (e.g., gd for the png and jpeg devices)
#   where at least one of the associated devices is enabled.

set(DRIVERS_LINK_FLAGS)
option(ENABLE_DYNDRIVERS "Enable dynamic loading of device drivers" OFF)
if(ENABLE_DYNDRIVERS AND NOT BUILD_SHARED_LIBS)
  message(STATUS
    "WARNING: Shared libraries not built. Setting ENABLE_DYNDRIVERS OFF."
    )
  set(ENABLE_DYNDRIVERS OFF CACHE BOOL
    "Enable dynamic loading of device drivers" FORCE)
endif(ENABLE_DYNDRIVERS AND NOT BUILD_SHARED_LIBS)
if(ENABLE_DYNDRIVERS AND WIN32_OR_CYGWIN)
  if(NOT CYGWIN)
    set(LTDL_WIN32 ON)
    set(LTDL_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/include)
  endif(NOT CYGWIN)
endif(ENABLE_DYNDRIVERS AND WIN32_OR_CYGWIN)
if(ENABLE_DYNDRIVERS AND NOT LTDL_WIN32)
  find_package(LTDL)
  if(LTDL_FOUND)
    set(LTDL_LIBRARY_RPATH ${LTDL_LIBRARY_DIR})
    filter_rpath(LTDL_LIBRARY_RPATH)
    message(STATUS "LTDL_INCLUDE_DIR = ${LTDL_INCLUDE_DIR}")
    message(STATUS "LTDL_LIBRARY_DIR = ${LTDL_LIBRARY_DIR}")
    message(STATUS "LTDL_LIBRARIES = ${LTDL_LIBRARIES}")
  else(LTDL_FOUND)
    message(STATUS
      "WARNING: libltdl library not found. Setting ENABLE_DYNDRIVERS OFF.\n"
      "   Please install that library and/or set the environment variables\n"
      "   CMAKE_INCLUDE_PATH and CMAKE_LIBRARY_PATH appropriately."
      )
    set(ENABLE_DYNDRIVERS OFF CACHE BOOL
      "Enable dynamic loading of device drivers" FORCE)
  endif(LTDL_FOUND)
endif(ENABLE_DYNDRIVERS AND NOT LTDL_WIN32)

# Needed to configure traditional build system for installed examples.
if(ENABLE_DYNDRIVERS)
  set(enable_dyndrivers_true "")
  set(enable_dyndrivers_false "#")
else(ENABLE_DYNDRIVERS)
  set(enable_dyndrivers_true "#")
  set(enable_dyndrivers_false "")
endif(ENABLE_DYNDRIVERS)

# Decide whether to enable each device or not and find special resources
# when required.

# Initialize device options (e.g., PLD_png is set to ON or OFF).
include(drivers-init)

# Find *_COMPILE_FLAGS and *_LINK_FLAGS resources for device drivers that need them,
# and set appropriate PLD_devicename to OFF if the required resources are
# not available.
include(cairo)
include(cgm)
include(gd)
include(xwin)
include(tk)
include(pstex)
include(psttf)
include(qt)
include(wingcc)
include(aqt)
include(wxwidgets)
include(pdf)
include(wingdi)

# Finalize device options.
include(drivers-finish)
