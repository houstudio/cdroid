# iwidgets4.0/CMakeLists.txt

# Configure the build of iwidgets4.0.

# Copyright (C) 2013 Alan W. Irwin

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

set(PACKAGE iwidgets4.0)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST itk3 itcl3 tk tcl)

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

set(VERSION 4.0.1)
set(LIBVERSION 4.0)
set(CMAKE_OPTION -DBUILD_IWIDGETS_4.1=OFF)

# Data that is related to downloads.
set(URL http://downloads.sourceforge.net/project/incrtcl/%5bIncr%20Widgets%5d/${VERSION}/iwidgets${VERSION}.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH 0e9c140e81ea6015b56130127c7deb03)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_CMAKE_COMMAND} ${CMAKE_OPTION} ${EPA_BASE}/Source/build_${PACKAGE}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND} install
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-patch
  COMMAND echo "Add CMake-based build system"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_SOURCE_DIR}/iwidgets/iwidgets_CMakeLists.txt
  ${EPA_BASE}/Source/build_${PACKAGE}/CMakeLists.txt
  APPEND
  )
