# xmlcatalog-wrapper/CMakeLists.txt

# Configure the build of xmlcatalog-wrapper.

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

# The top-level CMakeLists.txt file that adds this subdirectory should
# have run "include(ExternalProject)" and set the EP_BASE variable (and
# corresponding directory property) as well as various EPA variables
# used below that configure how the External_Project functions
# operate.

set(PACKAGE xmlcatalog-wrapper)
# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST pkg-config;bison;flex;python2-devel;libXft ${extra_ignored_dependencies_list})

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

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  # The entire source code is just one CMakeLists.txt file.
  DOWNLOAD_COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${PACKAGE}_CMakeLists.txt ${EPA_BASE}/Source/build_${PACKAGE}/CMakeLists.txt
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_CMAKE_COMMAND} ${EP_BASE}/Source/build_${PACKAGE}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND}
  BUILD_IN_SOURCE OFF
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_BUILD_COMMAND} install
  )
