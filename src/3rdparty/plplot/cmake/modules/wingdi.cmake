# cmake/modules/wingdi.cmake
#
# Copyright (C) 2006 Werner Smekal
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

# Module for determining all configuration variables related to the wingdi
# device driver.
# The following variables are set/modified:
# PLD_wingdi		  - ON means the wingdi device is enabled.
# wingdi_COMPILE_FLAGS	  - individual COMPILE_FLAGS required to compile wingdi
# 			    device.
# wingdi_LINK_FLAGS	  - list of pathnames of libraries for dynamic
# 			    wingdi device.
# DRIVERS_LINK_FLAGS	  - list of LINK_FLAGS for all static devices.

# Windows-only device driver.
if(PLD_wingdi AND NOT WIN32_OR_CYGWIN)
  set(PLD_wingdi OFF CACHE BOOL "Enable wingdi device" FORCE)
endif(PLD_wingdi AND NOT WIN32_OR_CYGWIN)

if(PLD_wingdi)
  message(STATUS "Looking for gdi32 header and library")
  find_library(GDI32_LIBRARY gdi32 HINTS ${MINGWLIBPATH} ${BORLANDLIBPATH})
  if(GDI32_LIBRARY)
    find_library(COMDLG32_LIBRARY comdlg32 HINTS ${MINGWLIBPATH} ${BORLANDLIBPATH})
	find_library(COMCTL32_LIBRARY comctl32 HINTS ${MINGWLIBPATH} ${BORLANDLIBPATH})
  endif(GDI32_LIBRARY)
  if(GDI32_LIBRARY AND COMDLG32_LIBRARY AND COMCTL32_LIBRARY)
    message(STATUS "Looking for gdi32 header and library - found")
    set(wingdi_LINK_FLAGS "${GDI32_LIBRARY};${COMDLG32_LIBRARY};${COMCTL32_LIBRARY}")
    set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${wingdi_LINK_FLAGS})
  else(GDI32_LIBRARY AND COMDLG32_LIBRARY AND COMCTL32_LIBRARY)
    message(STATUS "Looking for gdi32 header and library - not found")
    message(STATUS "WARNING: Setting PLD_wingdi to OFF.")
    set(PLD_wingdi OFF CACHE BOOL "Enable wingdi device" FORCE)
  endif(GDI32_LIBRARY AND COMDLG32_LIBRARY AND COMCTL32_LIBRARY)
endif(PLD_wingdi)
