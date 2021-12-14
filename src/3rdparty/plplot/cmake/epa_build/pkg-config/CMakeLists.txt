# pkg-config/CMakeLists.txt

# Configure the build of pkg-config.

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

set(PACKAGE pkg-config)

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

# Data that is related to downloads.
set(URL http://pkgconfig.freedesktop.org/releases/pkg-config-0.28.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH aa3c86e67551adc3ac865160e34a2a0d)


# Data that is related to the PATH that must be used.
if(MSYS_PLATFORM)
  # configure (for glib subpackage) dies without this.
  set(CFLAGS "-march=native $ENV{CFLAGS}")
  # These changes cause improved test results (three failures rather than 4)
  # on Wine, see <https://bugs.freedesktop.org/show_bug.cgi?id=66939>
  set(BUILD_IN_SOURCE 1)
  set(BUILD_COMMAND ${EPA_MAKE_COMMAND})
else(MSYS_PLATFORM)
  set(CFLAGS "$ENV{CFLAGS}")
  # No changes from defaults required to get perfect test results
  # on Linux.
  set(BUILD_IN_SOURCE 0)
  set(BUILD_COMMAND ${EPA_PARALLEL_MAKE_COMMAND})
endif(MSYS_PLATFORM)

# Drop -fvisibility=hidden since that option may not work for this package.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

ExternalProject_Add(
  build_${PACKAGE}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND} --with-internal-glib
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${BUILD_COMMAND}
  # 4 of 25 tests fail for the MinGW/MSYS/Wine platform so comment out
  # the next two lines for now.
  #TEST_BEFORE_INSTALL ON
  #TEST_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${BUILD_COMMAND} check
  BUILD_IN_SOURCE ${BUILD_IN_SOURCE}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${BUILD_COMMAND} install
  )

ExternalProject_Add_Step(
  build_${PACKAGE} preinstall_fix
  COMMENT "remove pkg-config binaries from the install tree"
  DEPENDEES build
  DEPENDERS install
  COMMAND ${CMAKE_COMMAND} -DFILE_WILDCARD:STRING=${EPA_CMAKE_INSTALL_PREFIX}/bin/*-pkg-config* -P ${CMAKE_SOURCE_DIR}/wildcard_remove.cmake
  )
