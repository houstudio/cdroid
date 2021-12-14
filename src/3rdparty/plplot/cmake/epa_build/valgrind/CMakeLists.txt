# valgrind/CMakeLists.txt

# Configure the build of valgrind.

# This latest version supports --num-callers up to 500 which
# is an important feature for certain critical cases.

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

set(PACKAGE valgrind)

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

# Drop -fvisibility=hidden since that option does not work for a
# number of software packages that are configured automatically using
# this template.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

# Data that is related to downloads.
set(URL http://valgrind.org/downloads/valgrind-3.10.1.tar.bz2)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH 60ddae962bc79e7c95cfc4667245707f)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  BUILD_IN_SOURCE @BUILD_IN_SOURCE@
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install
  )
