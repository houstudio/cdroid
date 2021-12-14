# cmake/modules/lua.cmake
#
# Lua binding configuration
#
# Copyright (C) 2008 Werner Smekal
# Copyright (C) 2017-2018 Alan W. Irwin
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

# Module for determining Lua binding configuration options

# Options to enable Lua binding
if(DEFAULT_NO_BINDINGS)
  option(ENABLE_lua "Enable Lua binding" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_lua "Enable Lua binding" ON)
endif(DEFAULT_NO_BINDINGS)

if(ENABLE_lua AND NOT BUILD_SHARED_LIBS)
  message(STATUS "WARNING: "
    "Lua requires shared libraries. Disabling Lua binding")
  set(ENABLE_lua OFF CACHE BOOL "Enable Lua binding" FORCE)
endif(ENABLE_lua AND NOT BUILD_SHARED_LIBS)

if(ENABLE_lua AND NOT SWIG_FOUND)
  message(STATUS "WARNING: "
    "swig not found. Disabling Lua binding")
  set(ENABLE_lua OFF CACHE BOOL "Enable Lua binding" FORCE)
endif(ENABLE_lua AND NOT SWIG_FOUND)

if(ENABLE_lua)
  if(${SWIG_VERSION} VERSION_LESS "1.3.35")
    message(STATUS "WARNING: swig version too old for Lua binding.   Disabling Lua binding.")
    set(ENABLE_lua OFF CACHE BOOL "Enable Lua binding" FORCE)
  endif(${SWIG_VERSION} VERSION_LESS "1.3.35")
endif(ENABLE_lua)

if(ENABLE_lua AND NOT PL_DOUBLE)
  message(STATUS "WARNING: "
    "Only single-precision floating point. Disabling Lua binding")
  set(ENABLE_lua OFF CACHE BOOL "Enable Lua binding" FORCE)
endif(ENABLE_lua AND NOT PL_DOUBLE)

if(ENABLE_lua)
  # Find Lua.  If the user specifies REQUIRED_LUA_VERSION, only that version
  # of Lua will be accepted.  Otherwise, the Lua version found will be the
  # highest version installed on your system.
  if(REQUIRED_LUA_VERSION)
    message(STATUS "Attempting to find specific Lua version = ${REQUIRED_LUA_VERSION}")
    find_package(Lua ${REQUIRED_LUA_VERSION} EXACT QUIET)
  else(REQUIRED_LUA_VERSION)
    message(STATUS "Attempting to find any Lua version")
    find_package(Lua QUIET)
  endif(REQUIRED_LUA_VERSION)
  message(STATUS "LUA_INCLUDE_DIR = ${LUA_INCLUDE_DIR}")
  message(STATUS "LUA_LIBRARIES = ${LUA_LIBRARIES}")
  if(NOT LUA_FOUND)
    message(STATUS "WARNING: "
      "Lua header and/or library not found. Disabling Lua binding")
    set(ENABLE_lua OFF CACHE BOOL "Enable Lua binding" FORCE)
  endif(NOT LUA_FOUND)
endif(ENABLE_lua)

if(ENABLE_lua)
  string(SUBSTRING ${LUA_VERSION_STRING} 0 3 SHORT_LUA_VERSION_STRING)
  # Look for consistently versioned LUA_EXECUTABLE and only use the
  # "lua" name for that executable as a last resort because the
  # generic system version may not correspond to the library that is
  # found. But in order to find a locally built version (if higher
  # than the system version on the search PATHs) must also use
  # NAMES_PER_DIR.

  find_program(LUA_EXECUTABLE NAMES lua${SHORT_LUA_VERSION_STRING} lua NAMES_PER_DIR)
  message(STATUS "Found LUA_EXECUTABLE = ${LUA_EXECUTABLE}")
  if(NOT LUA_EXECUTABLE)
    message(STATUS "WARNING: "
      "Lua executable not found under either lua${SHORT_LUA_VERSION_STRING} or lua name. Disabling Lua binding")
    set(ENABLE_lua OFF CACHE BOOL "Enable Lua binding" FORCE)
  endif(NOT LUA_EXECUTABLE)
endif(ENABLE_lua)

if(ENABLE_lua)
  # Unless some better convention comes along, follow what Debian does for
  # install location of Lua wrapper shared object.
  if(LUA_VERSION MATCHES "5\\.0")
    set(
      LUA_DIR
      ${LIB_DIR}/lua/50/plplot
      CACHE PATH "PLplot install location for Lua wrapper shared object"
      )
  else(LUA_VERSION MATCHES "5\\.0")
    set(
      LUA_DIR
      ${LIB_DIR}/lua/${LUA_VERSION}/plplot
      CACHE PATH "PLplot install location for Lua wrapper shared object"
      )
  endif(LUA_VERSION MATCHES "5\\.0")
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST LUA_DIR)
endif(ENABLE_lua)
