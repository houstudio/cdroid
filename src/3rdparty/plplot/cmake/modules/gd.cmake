# cmake/modules/gd.cmake
#
# Copyright (C) 2006  Andrew Ross
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

# Configuration for the gd device driver (supporting png, gif and/or jpeg
# devices.)
#
# The following variables are set / modified
#
# PLD_png             - ON means the png device is enabled
# PLD_jpeg            - ON means the jpeg device is enabled
# PLD_gif             - ON means the gif device is enabled
# gd_COMPILE_FLAGS    - COMPILE_FLAGS required to compile gd device driver.
# gd_LINK_FLAGS       - LINK_FLAGS for dynamic gd device driver.
# DRIVERS_LINK_FLAGS  - list of LINK_FLAGS for all static device drivers.
#
# Include file searches use FindPath. To add extra search directories
# set the environment variable CMAKE_INCLUDE_PATH.
# Library searches use FindLibrary. To add extra search directories
# set the environment variable CMAKE_LIBRARY_PATH.
# See cmake documentation for further details.

# Look for gd libraries
if(PLD_png OR PLD_jpeg OR PLD_gif)
  find_package(GD)
  message(STATUS "GD_INCLUDE_DIRS = ${GD_INCLUDE_DIRS}")
  message(STATUS "GD_LIBRARIES = ${GD_LIBRARIES}")
  message(STATUS "GD_LIBRARY_DIRS = ${GD_LIBRARY_DIRS}")
  if(NOT GD_FOUND)
    message(STATUS "WARNING: at least one of GD_INCLUDE_DIRS, GD_LIBRARIES, or GD_LIBRARY_DIRS is false so setting PLD_png, PLD_jpeg, and PLD_git to OFF")
    set(PLD_png OFF CACHE BOOL "Enable png device" FORCE)
    set(PLD_jpeg OFF CACHE BOOL "Enable jpeg device" FORCE)
    set(PLD_gif OFF CACHE BOOL "Enable gif device" FORCE)
  else(NOT GD_FOUND)

    if(WITH_FREETYPE)
      set(GD_INCLUDE_DIRS ${GD_INCLUDE_DIRS} ${FREETYPE_INCLUDE_DIRS})
      set(GD_LIBRARIES ${GD_LIBRARIES} ${FREETYPE_LIBRARIES})
    endif(WITH_FREETYPE)
  endif(NOT GD_FOUND)
endif(PLD_png OR PLD_jpeg OR PLD_gif)

if(GD_FOUND)
  if(NOT GD_SUPPORTS_PNG)
    set(PLD_png OFF CACHE BOOL "Enable png device" FORCE)
  endif(NOT GD_SUPPORTS_PNG)

  if(NOT GD_SUPPORTS_JPEG)
    set(PLD_jpeg OFF CACHE BOOL "Enable jpeg device" FORCE)
  endif(NOT GD_SUPPORTS_JPEG)

  if(NOT GD_SUPPORTS_GIF)
    set(PLD_gif OFF CACHE BOOL "Enable gif device" FORCE)
  endif(NOT GD_SUPPORTS_GIF)
endif(GD_FOUND)

if(PLD_png OR PLD_jpeg OR PLD_gif)
  # Convert directory list to compile flags
  set(gd_COMPILE_FLAGS)
  foreach(DIR ${GD_INCLUDE_DIRS})
    set(gd_COMPILE_FLAGS "${gd_COMPILE_FLAGS} -I${DIR}")
  endforeach(DIR ${GD_INCLUDE_DIRS})

  # Generate library link commands
  set(gd_LINK_FLAGS ${GD_LIBRARIES})
  set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${gd_LINK_FLAGS})
endif(PLD_png OR PLD_jpeg OR PLD_gif)

