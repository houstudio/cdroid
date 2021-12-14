# libqhull/CMakeLists.txt

# Configure the build of libqhull.

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

set(PACKAGE libqhull)

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
set(URL http://www.qhull.org/download/qhull-2010.1-src.tgz)
set(URL_MD5 e64138470acdeb18f752a0bc2a11ceb4)

ExternalProject_Add(
  build_${PACKAGE}
  URL ${URL}
  URL_MD5 ${URL_MD5}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${EPA_CMAKE_COMMAND} ${EPA_BASE}/Source/build_${PACKAGE}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
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
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt
  ${EPA_BASE}/Source/build_${PACKAGE}/src/CMakeLists.txt
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/src/${PACKAGE}.h
  ${EPA_BASE}/Source/build_${PACKAGE}/src/${PACKAGE}.h
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mem.h
  ${EPA_BASE}/Source/build_${PACKAGE}/src/mem.h
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/src/unix.c
  ${EPA_BASE}/Source/build_${PACKAGE}/src/unix.c
  COMMENT "Custom updating of ${PACKAGE}"
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}_CMakeLists.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/src/CMakeLists.txt
  ${CMAKE_CURRENT_SOURCE_DIR}/src/${PACKAGE}.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/mem.h
  ${CMAKE_CURRENT_SOURCE_DIR}/src/unix.c
  APPEND
  )
