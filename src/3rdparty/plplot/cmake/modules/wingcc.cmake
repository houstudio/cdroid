# cmake/modules/wingcc.cmake
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

# Module for determining all configuration variables related to the wingcc
# device driver.
# The following variables are set/modified:
# PLD_wingcc		  - ON means the wingcc device is enabled.
# wingcc_COMPILE_FLAGS	  - individual COMPILE_FLAGS required to compile wingcc
# 			    device.
# wingcc_LINK_FLAGS	  - list of pathnames of libraries for dynamic
# 			    wingcc device.
# DRIVERS_LINK_FLAGS	  - list of LINK_FLAGS for all static devices.

# Windows-only device driver.
if(PLD_wingcc AND NOT WIN32_OR_CYGWIN)
  set(PLD_wingcc OFF CACHE BOOL "Enable wingcc device" FORCE)
endif(PLD_wingcc AND NOT WIN32_OR_CYGWIN)

if(PLD_wingcc)
  message(STATUS "Looking for gdi32 header and library")
  find_library(GDI32_LIBRARY gdi32 HINTS ${MINGWLIBPATH} ${BORLANDLIBPATH})
  if(GDI32_LIBRARY)
    find_library(COMDLG32_LIBRARY comdlg32 HINTS ${MINGWLIBPATH} ${BORLANDLIBPATH})
  endif(GDI32_LIBRARY)
  if(GDI32_LIBRARY AND COMDLG32_LIBRARY)
    message(STATUS "Looking for gdi32 header and library - found")
    set(wingcc_LINK_FLAGS "${GDI32_LIBRARY};${COMDLG32_LIBRARY}")
    if(WITH_FREETYPE)
      set(
      wingcc_COMPILE_FLAGS
      "${wingcc_COMPILE_FLAGS} ${FREETYPE_INCLUDE_CFLAGS}"
      )
      set(
      wingcc_LINK_FLAGS
      ${wingcc_LINK_FLAGS}
      ${FREETYPE_LIBRARIES}
      )
    endif(WITH_FREETYPE)
    set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${wingcc_LINK_FLAGS})
  else(GDI32_LIBRARY AND COMDLG32_LIBRARY)
    message(STATUS "Looking for gdi32 header and library - not found")
    message(STATUS "WARNING: Setting PLD_wingcc to OFF.")
    set(PLD_wingcc OFF CACHE BOOL "Enable wingcc device" FORCE)
  endif(GDI32_LIBRARY AND COMDLG32_LIBRARY)
endif(PLD_wingcc)
