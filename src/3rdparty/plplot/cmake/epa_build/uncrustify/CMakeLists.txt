# uncrustify/CMakeLists.txt

# Configure the build of uncrustify.

# Copyright (C) 2015 Alan W. Irwin

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

set(PACKAGE uncrustify)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST)

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

# Data that is related to downloads.
# To find SourceForge downloads version replace
# http://sourceforge.net/projects/SOFTWARE_PACKAGE/files prefix with
# http://downloads.sourceforge.net/project/SOFTWARE_PACKAGE prefix.
set(PACKAGE_VERSION ${PACKAGE}-0.60)
set(URL http://downloads.sourceforge.net/${PACKAGE}/${PACKAGE}/${PACKAGE_VERSION}/${PACKAGE_VERSION}.tar.gz)
set(URL_MD5 0467a60b7c6f108259e69a90120afd4a)

ExternalProject_Add(
  build_${PACKAGE}
  URL ${URL}
  URL_MD5 ${URL_MD5}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${EPA_CMAKE_COMMAND} ${EPA_BASE}/Source/build_${PACKAGE}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
  #TEST_BEFORE_INSTALL ON
  #TEST_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_CTEST_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND} install
  )

# Add custom commands to the current no-command update step.
add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-update
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}_CMakeLists.txt
  ${EPA_BASE}/Source/build_${PACKAGE}/CMakeLists.txt
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  ${EPA_BASE}/Source/build_${PACKAGE}/config.h.cmake
  COMMENT "Custom updating of ${PACKAGE}"
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}_CMakeLists.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/config.h.cmake
  APPEND
  )
