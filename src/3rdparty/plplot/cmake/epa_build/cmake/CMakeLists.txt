# cmake/CMakeLists.txt

# Configure the build of cmake.

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

# An external curl library (see --system-curl option below) is
# required to build cmake because the cmake internal version of curl
# is quite lame (no openssl capability, for example).  Also, we
# haven't yet implemented a curl epa_build configuration because that
# is going to be somewhat difficult due to the large number of
# dependencies.  See
# http://www.linuxfromscratch.org/blfs/view/svn/basicnet/curl.html for
# a list of those dependencies.  There is some hope because
# some of those are optional so I may tackle the curl epa_build configuration
# sooner rather than later.

# For now we are okay on Linux because we can just use the system curl
# library for the cmake build, but on Windows it is unlikely curl is
# installed so we will be unable to build a powerful cmake (until a
# curl epa_build configuration is available).  Note, if the Windows
# build of cmake fails, then either install curl (if that is possible)
# or use the workaround of downloading a binary download from kitware
# that apparently does use a powerful Windows version of curl.

set(PACKAGE cmake)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST curl ${extra_ignored_dependencies_list})

set(dependencies_LIST curl)

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

# Data that is related to downloads.
set(DIR v3.0)
set(VERSION 3.0.2)
set(URL http://www.cmake.org/files/${DIR}/cmake-${VERSION}.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH db4c687a31444a929d2fdc36c4dfb95f)

ExternalProject_Add(
  build_${PACKAGE}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" "CXXFLAGS=${CXXFLAGS}" ${SH_EXECUTABLE} ${EPA_BASE}/Source/build_${PACKAGE}/bootstrap --prefix=${EPA_CMAKE_INSTALL_PREFIX} --parallel=8 --verbose --system-curl --no-qt-gui
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND} install
  )
