# cmake/modules/java.cmake
#
# Java binding configuration
#
# Copyright (C) 2006 Andrew Ross
# Copyright (C) 2017 Alan W. Irwin
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

# Module for determining Java binding configuration options

# Options to enable Java binding
if(DEFAULT_NO_BINDINGS)
  option(ENABLE_java "Enable Java binding" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_java "Enable Java binding" ON)
endif(DEFAULT_NO_BINDINGS)

if(ENABLE_java AND NOT BUILD_SHARED_LIBS)
  message(STATUS "WARNING: "
    "Java requires shared libraries. Disabling java binding")
  set(ENABLE_java OFF CACHE BOOL "Enable Java binding" FORCE)
endif(ENABLE_java AND NOT BUILD_SHARED_LIBS)

if(ENABLE_java AND NOT SWIG_FOUND)
  message(STATUS "WARNING: "
    "swig not found. Disabling Java binding")
  set(ENABLE_java OFF CACHE BOOL "Enable Java binding" FORCE)
endif(ENABLE_java AND NOT SWIG_FOUND)

if(ENABLE_java AND NOT PLPLOT_Java_COMPILER_WORKS)
  workaround_9220(Java PLPLOT_Java_COMPILER_WORKS)
  if(NOT PLPLOT_Java_COMPILER_WORKS)
    message(STATUS "WARNING: no working Java compiler so disabling Java binding and examples.")
    set(ENABLE_java OFF CACHE BOOL "Enable Java binding" FORCE)
  endif(NOT PLPLOT_Java_COMPILER_WORKS)
endif(ENABLE_java AND NOT PLPLOT_Java_COMPILER_WORKS)

if(ENABLE_java AND NOT PL_DOUBLE)
  message(STATUS "WARNING: "
    "Only single-precision floating point. Disabling Java binding because of build errors in this case.")
  set(ENABLE_java OFF CACHE BOOL "Enable Java binding" FORCE)
endif(ENABLE_java AND NOT PL_DOUBLE)

if(ENABLE_java)
  # Find and check Java compiler.
  enable_language(Java OPTIONAL)
  if(NOT CMAKE_Java_COMPILER_WORKS)
    message(STATUS "WARNING: no working Java compiler so disabling Java binding and examples.")
    set(ENABLE_java OFF CACHE BOOL "Enable java binding" FORCE)
  endif(NOT CMAKE_Java_COMPILER_WORKS)
endif(ENABLE_java)

if(ENABLE_java)
  if(NOT CMAKE_Java_ARCHIVE)
    message(STATUS "WARNING: java archiver (jar) not found. Disabling java binding")
    set(ENABLE_java OFF CACHE BOOL "Enable Java binding" FORCE)
  endif(NOT CMAKE_Java_ARCHIVE)
endif(ENABLE_java)

if(ENABLE_java)
  # Check for java environment
  find_package(JNI)
  # If CMake doesn't find jni.h you need set CMAKE_INCLUDE_PATH
  if(NOT JAVA_INCLUDE_PATH)
    message(STATUS
      "WARNING: jni.h header not found. Disabling Java binding.\n"
      "   Please install that header and/or set the environment variable\n"
      "   CMAKE_INCLUDE_PATH appropriately."
      )
    set(ENABLE_java OFF CACHE BOOL "Enable Java binding" FORCE)
  endif(NOT JAVA_INCLUDE_PATH)
endif(ENABLE_java)

if(ENABLE_java)
  set(
    JAR_DIR
    ${CMAKE_INSTALL_DATADIR}/java
    CACHE PATH "PLplot jar file install location"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST JAR_DIR)

  set(
    JAVAWRAPPER_DIR
    ${LIB_DIR}/jni
    CACHE PATH "PLplot java plugin (JNI *.so) install location"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST JAVAWRAPPER_DIR)

  # These "HARDDIR" directory locations are designed to be used for
  # hard-coded java install locations in configured files which should
  # refer to the final absolute install location rather than some
  # intermediate staging area location for the install that is
  # typically used by package builders.

  # These variables are uncached (i.e., not settable by users) since
  # they should be exactly consistent with the absolute locations
  # corresponding to JAR_DIR and JAVAWRAPPER_DIR

  get_filename_component(JAR_HARDDIR ${JAR_DIR} ABSOLUTE)
  get_filename_component(JAVAWRAPPER_HARDDIR ${JAVAWRAPPER_DIR} ABSOLUTE)
endif(ENABLE_java)

# Set up java compiler flags and turn into a list suitable for add_custom_command
set(DEFAULT_JAVACFLAGS "")
set(JAVACFLAGS ${DEFAULT_JAVACFLAGS} CACHE STRING "Java compiler flags")
string(REGEX REPLACE " " ";" JAVACFLAGS "${JAVACFLAGS}")
