# libharu/CMakeLists.txt

# Configure the build of libharu (a.k.a., libhpdf).

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

set(PACKAGE libharu)

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
set(URL http://${PACKAGE}.org/files/${PACKAGE}-2.2.1.tar.gz)
set(URL_MD5 63da8e087744f1d6cf16f9501b1cb766)

ExternalProject_Add(
  build_${PACKAGE}
  URL ${URL}
  URL_MD5 ${URL_MD5}
  PATCH_COMMAND ""
  # Do not use -DLIBHARU_EXAMPLES=ON (which builds the demos, but does
  # not test them) because latest release does not include demos.
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${EPA_CMAKE_COMMAND} ${EPA_BASE}/Source/build_${PACKAGE}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND} install
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-patch
  COMMAND ${CMAKE_COMMAND} -E echo "Apply all patches"
  COMMAND ${PATCH_EXECUTABLE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/cmake.patch
  COMMAND ${PATCH_EXECUTABLE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/large_font.patch
  COMMAND ${PATCH_EXECUTABLE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/remove_configured.patch
  COMMAND ${PATCH_EXECUTABLE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/visibility.patch
  DEPENDS
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/large_font.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/remove_configured.patch
  ${CMAKE_CURRENT_SOURCE_DIR}/visibility.patch
  APPEND
  )
