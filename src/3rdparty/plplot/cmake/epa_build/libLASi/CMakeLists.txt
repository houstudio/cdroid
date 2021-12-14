# libLASi/CMakeLists.txt

# Configure the build of a "lite" version of PLplot (with fewer dependencies
# but with fewer features as well).

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

set(PACKAGE libLASi)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST
  freetype
  pango
  )

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

set(CXXFLAGS "$ENV{CXXFLAGS}")

# Data that is related to downloads.

# Convert download URL you get by mouse clicking on link =
# http://sourceforge.net/projects/lasi/files/lasi/1.1.2%20Source/libLASi-1.1.2.tar.gz/download
# using the usual Sourceforge transformation.
set(URL http://downloads.sourceforge.net/project/lasi/lasi/1.1.2%20Source/libLASi-1.1.2.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH 29de80a4878aa71d497c7ae637c5d9a8)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS "${dependencies_targets}"
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CXXFLAGS=${CXXFLAGS}" ${EPA_CMAKE_COMMAND} ${EPA_BASE}/Source/build_${PACKAGE}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND} install
  TEST_AFTER_INSTALL 1
  TEST_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_CTEST_COMMAND}
  )
