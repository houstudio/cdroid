# iwidgets/CMakeLists.txt

# Configure the build of iwidgets.

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

set(PACKAGE iwidgets)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST itk tk tcl)

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

set(VERSION 4.1.0)
set(OLD_VERSION 4.0.1)
set(LIBVERSION 4.1)
set(CMAKE_OPTION -DBUILD_IWIDGETS_4.1=ON)

# Data that is related to downloads.
set(URL http://downloads.sourceforge.net/project/incrtcl/%5bIncr%20Widgets%5d/${OLD_VERSION}/iwidgets${OLD_VERSION}.tar.gz)
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
# For this (4.1) case we do not use the
# http://downloads.sourceforge.net/project/kbskit/itk/iwidgets41.tar.gz
# tarball because there are some problems (incorrect install
# locations and only a subset of what is normally installed) for
# that binary distribution of iwidgets4.1.  Instead, we build 4.1
# similarly to 4.0 except we apply the following patch which was
# derived as follows:
#
# diff -Naur iwidgets4.0.1/README iwidgets4.1/README >iwidgets4.1.patch
# diff -Naur iwidgets4.0.1/license.terms iwidgets4.1/license.terms >>iwidgets4.1.patch
# diff -Naur --exclude=pkgIndex.tcl iwidgets4.0.1/generic iwidgets4.1/library >> iwidgets4.1.patch
# diff -Naur iwidgets4.0.1/tests iwidgets4.1/tests >> iwidgets4.1.patch
#
# N.B. this patch is somewhat larger than
# iwidgets4.1/iwidgets.patch because that latter patch is only
# concerned with generic subdirectory differences and it also
# appears to be taken against iwidgets-4.0.2 (which I cannot find
# anywhere on the net) rather than iwidgets-4.0.1.
add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-patch
  COMMAND echo "Apply iwidgets4.1 source code patch"
  COMMAND ${PATCH_EXECUTABLE} --directory=${EPA_BASE}/Source/build_${PACKAGE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}4.1.patch
  COMMAND echo "Apply patch to support both iwidgets and Iwidgets package names"
  COMMAND ${PATCH_EXECUTABLE} --directory=${EPA_BASE}/Source/build_${PACKAGE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}4_case.patch
  APPEND
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-patch
  COMMAND echo "Add CMake-based build system"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
  ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}_CMakeLists.txt
  ${EPA_BASE}/Source/build_${PACKAGE}/CMakeLists.txt
  APPEND
  )
