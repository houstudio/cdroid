# itk3/CMakeLists.txt

# Configure build of itk3.

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

# These build configuration details for itk3 adapted from
# the fink build
# <http://www.mail-archive.com/fink-commits@lists.sourceforge.net/msg113511.html>

set(PACKAGE itk3)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST itcl3 tk tcl)

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
# Drop -fvisibility=hidden since that option does not work for itk3.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

# This variable provides the opportunity to set an alternative install
# prefix to distinguish what files are installed by version 3 of itcl
# and friends from what is installed by version 4 of itcl and friends.
# But currently just use the ordinary install location.
set(INCRTCL3_PREFIX ${EPA_CMAKE_INSTALL_PREFIX})

set(VERSION 3.3)
set(DIRVERSION 3.3)
set(ITCL3_DIRVERSION 3.4)
if(MSYS_PLATFORM)
  string(REGEX REPLACE "\\." "" LIBVERSION ${DIRVERSION})
else(MSYS_PLATFORM)
  set(LIBVERSION ${DIRVERSION})
endif(MSYS_PLATFORM)

# Data that is related to downloads.
set(URL http://downloads.sourceforge.net/project/incrtcl/%5bIncr%20Tcl_Tk%5d-source/${VERSION}-RC/itk${VERSION}.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH a97c17f3cfa5e377f43073c653c501b5)

if(EPA_HAVE_64_BIT_OS)
  set(ENABLE_64_BIT --enable-64bit)
endif(EPA_HAVE_64_BIT_OS)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  # Order of CPPFLAGS is important since want to access the version 3
  # itcl header.
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" "CPPFLAGS=-I${INCRTCL3_PREFIX}/include/itcl${ITCL3_DIRVERSION} -I${EPA_CMAKE_INSTALL_PREFIX}/include" ${source_PATH}/configure --prefix=${INCRTCL3_PREFIX} --exec-prefix=${INCRTCL3_PREFIX} --includedir=${INCRTCL3_PREFIX}/include/itcl${ITCL3_DIRVERSION} --mandir=${INCRTCL3_PREFIX}/share/man ${ENABLE_64_BIT} --with-tcl=${EPA_CMAKE_INSTALL_PREFIX}/lib --with-tk=${EPA_CMAKE_INSTALL_PREFIX}/lib --with-itcl=${INCRTCL3_PREFIX}/lib/itcl${ITCL3_DIRVERSION}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-patch
  COMMAND ${CMAKE_COMMAND} -E echo
  "Apply a patch for backporting the autotools changes for itk (version 4.0.0) to itk3"
  # These changes are replacing the old contents of the tclconfig
  # directory with the modern itk versions of those files and small
  # edits to the itk3 versions of configure.in. In addition the
  # "autoreconf -i" command was run on Linux to propagate these
  # changes to the corresponding configure shell script.  This could
  # not be done for each platform since the autoreconf command is
  # broken on MSYS.  A final change in this patch is Makefile.in was
  # edited to make it compatible with the itk version, and finally an
  # additional edit was made to this file so that CPPFLAGS and CFLAGS
  # take precedence over CONFIGS which contains the values derived
  # from the tclConfig.sh, tkConfig.sh, and itclConfig.sh files.
  # Without this final change the -I options come out in the wrong
  # order so that the itcl4 version of headers is (incorrectly) used
  # rather than the itcl3 version provided by CPPFLAGS.
  COMMAND ${PATCH_EXECUTABLE} --directory=${EPA_BASE}/Source/build_${PACKAGE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/autotools_backport.patch
  APPEND
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-build
  COMMAND echo "Replace build-tree locations by install-tree locations"
  COMMAND ${SED_EXECUTABLE}
  # Cover two different patterns of uppercasing.
  -e "s@^\\(itk_SRC_DIR='\\).*@\\1${INCRTCL3_PREFIX}/include/itcl${ITCL3_DIRVERSION}'@"
  -e "s@^\\(ITK_SRC_DIR='\\).*@\\1${INCRTCL3_PREFIX}/include/itcl${ITCL3_DIRVERSION}'@"
  # Cover two different patterns of uppercasing.
  -e "/itk_B/s@='\\(-L\\)\\?.*build_itk3@='\\1${INCRTCL3_PREFIX}/lib/itk${DIRVERSION}@"
  -e "/ITK_B/s@='\\(-L\\)\\?.*build_itk3@='\\1${INCRTCL3_PREFIX}/lib/itk${DIRVERSION}@"
  # Overwrite original file.
  -i itkConfig.sh
  APPEND
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND echo "Install-tree fixups"
  COMMAND ${CHMOD_EXECUTABLE} -v ${SO_NUMERICAL_PERMISSIONS} ${INCRTCL3_PREFIX}/lib/itk${DIRVERSION}/${library_prefix}itk${LIBVERSION}${library_suffix}
  APPEND
  )
