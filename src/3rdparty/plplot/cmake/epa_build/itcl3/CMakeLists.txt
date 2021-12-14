# itcl3/CMakeLists.txt

# Configure build of itcl3.

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

# These build configuration details for itcl3 adapted from
# the fink build
# <http://www.mail-archive.com/fink-commits@lists.sourceforge.net/msg113511.html>

set(PACKAGE itcl3)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST tk tcl)

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

if(MSYS_PLATFORM)
  set(library_prefix)
  set(library_suffix .dll)
else(MSYS_PLATFORM)
  set(library_prefix lib)
  set(library_suffix .so)
endif(MSYS_PLATFORM)

set(CFLAGS "$ENV{CFLAGS}")
# Drop -fvisibility=hidden since that option does not for itcl3.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

# This variable provides the opportunity to set an alternative install
# prefix to distinguish what files are installed by version 3 of itcl
# and friends from what is installed by version 4 of itcl and friends.
# But currently just use the ordinary install location.
set(INCRTCL3_PREFIX ${EPA_CMAKE_INSTALL_PREFIX})

set(VERSION 3.4.1)
string(REGEX REPLACE "\\.[0-9]$" "" DIRVERSION ${VERSION})
if(MSYS_PLATFORM)
  string(REGEX REPLACE "\\." "" LIBVERSION ${DIRVERSION})
else(MSYS_PLATFORM)
  set(LIBVERSION ${DIRVERSION})
endif(MSYS_PLATFORM)

# Data that is related to downloads.
set(URL http://downloads.sourceforge.net/project/incrtcl/%5bIncr%20Tcl_Tk%5d-source/${VERSION}/itcl${VERSION}.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH e7c98e0f69df1a675073ddd3344f0b7f)

if(EPA_HAVE_64_BIT_OS)
  set(ENABLE_64_BIT --enable-64bit)
endif(EPA_HAVE_64_BIT_OS)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${source_PATH}/configure --prefix=${INCRTCL3_PREFIX} --exec-prefix=${INCRTCL3_PREFIX} --includedir=${INCRTCL3_PREFIX}/include/itcl${DIRVERSION} --mandir=${INCRTCL3_PREFIX}/share/man ${ENABLE_64_BIT} --with-tcl=${EPA_CMAKE_INSTALL_PREFIX}/lib --with-tclinclude=${EPA_CMAKE_INSTALL_PREFIX}/include
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-build
  COMMAND echo "Replace build-tree locations by install-tree locations"
  COMMAND ${SED_EXECUTABLE}
  # Cover two different patterns of uppercasing.
  -e "s@^\\(itcl_SRC_DIR='\\).*@\\1${INCRTCL3_PREFIX}/include/itcl${DIRVERSION}'@"
  -e "s@^\\(ITCL_SRC_DIR='\\).*@\\1${INCRTCL3_PREFIX}/include/itcl${DIRVERSION}'@"
  # Cover two different patterns of uppercasing.
  -e "/itcl_B/s@='\\(-L\\)\\?.*build_itcl3@='\\1${INCRTCL3_PREFIX}/lib/itcl${DIRVERSION}@"
  -e "/ITCL_B/s@='\\(-L\\)\\?.*build_itcl3@='\\1${INCRTCL3_PREFIX}/lib/itcl${DIRVERSION}@"
  # Overwrite original file.
  -i itclConfig.sh
  APPEND
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND ${CMAKE_COMMAND} -E echo "Install-tree fixups"
  COMMAND ${CHMOD_EXECUTABLE} -v ${SO_NUMERICAL_PERMISSIONS} ${INCRTCL3_PREFIX}/lib/itcl${DIRVERSION}/${library_prefix}itcl${LIBVERSION}${library_suffix}
  COMMAND ${CMAKE_COMMAND} -E rename ${INCRTCL3_PREFIX}/lib/itclConfig.sh ${INCRTCL3_PREFIX}/lib/itcl${DIRVERSION}/itclConfig.sh
  APPEND
  )
