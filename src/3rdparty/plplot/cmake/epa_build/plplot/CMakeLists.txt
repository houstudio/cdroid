# plplot/CMakeLists.txt

# Configure the build of PLplot.

# Copyright (C) 2013-2015 Alan W. Irwin

# This file is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2.1 of the License, or (at your option) any later version.

# This file is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this file; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA

set(PACKAGE plplot)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST
  # agg website now (2014-04-13) a broken link, and this library only
  # needed in any case for a purpose that may disappear (wxwidgets).
  # So drop it.
  #libagg
  libharu
  libqhull
  shapelib
  wxwidgets
  )

# For now only add in these dependencies for the Unix case because
# these epa_builds do not yet work on Windows.

if(NOT MSYS_PLATFORM)
  list(APPEND dependencies_LIST
    pango
    qt5_lite
    libLASi
    octave_lite
    )
endif(NOT MSYS_PLATFORM)

# Do boilerplate tasks that must be done for each different project
# that is configured as part of epa_build.
epa_boilerplate(
  ignored_dependencies_LIST
  PACKAGE
  dependencies_LIST
  dependencies_targets
  EPA_PATH
  source_PATH
  )

set(CFLAGS "$ENV{CFLAGS}")
set(CXXFLAGS "$ENV{CXXFLAGS}")
set(FFLAGS "$ENV{FFLAGS}")

set(cmake_args)

# Drop gtk+2-only dependencies since those clash (at build time for static
# device drivers and at run-time with dynamic device drivers) with
# gtk+3 that is used with epa_build.
list(APPEND cmake_args
  -DDROP_GTK_PLUS_2_BUILDS=ON
  )

# Only use Qt5 since that is all that is epa_built.
list(APPEND cmake_args
  -DPLPLOT_USE_QT5=ON
  )

if(MSYS_PLATFORM)
  # Turn off the cairo and qt device drivers for plplot on Windows
  # until the pango and qt5_lite packages and all their dependencies
  # build properly on that platform
  list(APPEND cmake_args
    # Except as a trial possibility we use a binary downloaded version
    # of gtk+ on Windows.
    #-DDEFAULT_NO_CAIRO_DEVICES:BOOL=ON
    -DDEFAULT_NO_QT_DEVICES:BOOL=ON
    -DENABLE_qt=OFF
    # Also turn off simple testing of dynamically loaded devices
    # because on Windows sometimes the simple test fails (in the
    # sense of giving the correct result while generating a
    # non-zero return code for unknown reasons) but the
    # actual dynload done by the PLplot library succeeds for some reason.
    -DTEST_DYNDRIVERS=OFF
    )
endif(MSYS_PLATFORM)

# Assumption that the top-level local PLplot source tree is two directories
# up from the present top-level directory for epa_build.
# This assumption is correct if you are accessing epa_build as
# a subset of the PLplot source tree so that its location is in
# cmake/epa_build within that source tree.
# But it is not the case if you have independently
# checked out just the epa_build subset of the normal PLplot source
# tree so check that the result really is a plplot source tree.
get_filename_component(PLPLOT_LOCAL_SOURCE_DIR ${CMAKE_SOURCE_DIR} PATH)
get_filename_component(PLPLOT_LOCAL_SOURCE_DIR ${PLPLOT_LOCAL_SOURCE_DIR} PATH)
find_file(IS_PLPLOT_SOURCE_TREE plcore.c
  HINTS ${PLPLOT_LOCAL_SOURCE_DIR}/src
  NO_DEFAULT_PATH
  )

if(NOT IS_PLPLOT_SOURCE_TREE)
  message(FATAL_ERROR "epa_build not located in cmake/epa_build in a PLplot source tree")
endif(NOT IS_PLPLOT_SOURCE_TREE)

# Data that is related to the PATH that must be used.
if(MSYS_PLATFORM)
  set(EPA_PATH_NODLL "${EPA_PATH}")
  set(EPA_PATH "${EPA_BASE}/Build/build_${PACKAGE}/dll;${EPA_PATH_NODLL}")
  determine_msys_path(EPA_PATH "${EPA_PATH}")
endif(MSYS_PLATFORM)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS "${dependencies_targets}"
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy_directory ${PLPLOT_LOCAL_SOURCE_DIR} ${EPA_BASE}/Source/build_${PACKAGE}
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
  INSTALL_COMMAND ""
  TEST_BEFORE_INSTALL OFF
  TEST_COMMAND ""
  STEP_TARGETS configure build install test
  )

if(COMPREHENSIVE_PLPLOT_TEST_INTERACTIVE OR COMPREHENSIVE_PLPLOT_TEST_NONINTERACTIVE)
  string(REGEX REPLACE ";" " " blank_delimited_cmake_args "${cmake_args}")

  if(COMPREHENSIVE_PLPLOT_TEST_INTERACTIVE)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/interactive_comprehensive_test.sh
      "echo yes | ${EPA_BASE}/Source/build_${PACKAGE}/scripts/comprehensive_test.sh --generator_string \"${CMAKE_GENERATOR}\" --cmake_added_options \"${blank_delimited_cmake_args}\" --build_command \"${EPA_BUILD_COMMAND} -j${NUMBER_PARALLEL_JOBS}\" --ctest_command \"${CMAKE_CTEST_COMMAND} -j${NUMBER_PARALLEL_JOBS} --timeout 15000 \" --do_ctest no --do_test_noninteractive no"
      )

    add_custom_command(
      OUTPUT
      ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-test
      COMMAND ${CMAKE_COMMAND} -E echo "comprehensive interactive tests"
      COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/interactive_comprehensive_test.sh
      APPEND
      )
  endif(COMPREHENSIVE_PLPLOT_TEST_INTERACTIVE)

  if(COMPREHENSIVE_PLPLOT_TEST_NONINTERACTIVE)
    file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/noninteractive_comprehensive_test.sh
      "echo yes | ${EPA_BASE}/Source/build_${PACKAGE}/scripts/comprehensive_test.sh --generator_string \"${CMAKE_GENERATOR}\" --cmake_added_options \"${blank_delimited_cmake_args}\" --build_command \"${EPA_BUILD_COMMAND} -j${NUMBER_PARALLEL_JOBS}\" --ctest_command \"${CMAKE_CTEST_COMMAND} -j${NUMBER_PARALLEL_JOBS} --timeout 15000 \" --do_test_interactive no"
      )

    add_custom_command(
      OUTPUT
      ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-test
      COMMAND ${CMAKE_COMMAND} -E echo "comprehensive noninteractive tests"
      COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/noninteractive_comprehensive_test.sh
      APPEND
      )
  endif(COMPREHENSIVE_PLPLOT_TEST_NONINTERACTIVE)
else(COMPREHENSIVE_PLPLOT_TEST_INTERACTIVE OR COMPREHENSIVE_PLPLOT_TEST_NONINTERACTIVE)

  # If ENABLE_COMPREHENSIVE_PLPLOT_TEST is not true, then just
  # configure a pure default case and build and install it.

  add_custom_command(
    OUTPUT
    ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-configure
    COMMAND ${CMAKE_COMMAND} -E echo "configure"
    COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" "CXXFLAGS=${CXXFLAGS}" "FFLAGS=${FFLAGS}" ${EPA_CMAKE_COMMAND} -DBUILD_TEST=ON ${cmake_args} ${EPA_BASE}/Source/build_${PACKAGE}
    APPEND
    )

  add_custom_command(
    OUTPUT
    ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-build
    COMMAND ${CMAKE_COMMAND} -E echo "build"
    COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
    APPEND
    )

  add_custom_command(
    OUTPUT
    ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
    COMMAND ${CMAKE_COMMAND} -E echo "install"
    COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND} install
    APPEND
    )

endif(COMPREHENSIVE_PLPLOT_TEST_INTERACTIVE OR COMPREHENSIVE_PLPLOT_TEST_NONINTERACTIVE)
