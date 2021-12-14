# subversion/CMakeLists.txt

# Configure the build of subversion

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

set(PACKAGE subversion)
message(FATAL_ERROR "Incomplete build configuration")

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
# Drop -fvisibility=hidden since that option may not work for this package.
#string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

# Data that is related to downloads.
set(URL http://apache.parentingamerica.com/subversion/subversion-1.7.10.tar.bz2)
# TEMPORARY local version for debugging
set(URL /home/software/${PACKAGE}/${PACKAGE}-1.7.10.tar.bz2)
set(URL_MD5 4088a77e14232876c9b4ff1541e6e200)

ExternalProject_Add(
  build_${PACKAGE}
  URL ${URL}
  URL_MD5 ${URL_MD5}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install
  TEST_BEFORE_INSTALL OFF
  TEST_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} check
  )

# Add custom command to run the special get-deps.sh script
# to update the source tree with the required
# dependent library sources to be included in the subversion
# build that are recommended by the subversion project.
ExternalProject_Add_Step(
  build_${PACKAGE} download_extra_source
  COMMENT "Custom updating of ${PACKAGE} with downloaded source code"
  DEPENDEES download
  DEPENDERS configure
  COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${BASH_EXECUTABLE} get-deps.sh
  # N.B. no file dependencies are worthwhile since all data
  # are automatically downloaded.
  WORKING_DIRECTORY "${EPA_BASE}/Source/build_${PACKAGE}"
  )

# Add mkdir fix noted at
# http://mail-archives.apache.org/mod_mbox/subversion-users/201109.mbox/<838A6C12-013C-43AF-9645-A876CB0C5849@me.com>
# Why in the world is this simple fix not yet propagated upstream?
ExternalProject_Add_Step(
  build_${PACKAGE} mkdir_fix
  COMMENT "Fix 'mkdir serf/auth' issue"
  DEPENDEES configure
  DEPENDERS build
  COMMAND mkdir serf/auth
  WORKING_DIRECTORY "${EPA_BASE}/Build/build_${PACKAGE}"
  )
