# itk/CMakeLists.txt

# Configure build of itk.

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

# These build configuration details for itk adapted from
# the fink build
# <http://www.mail-archive.com/fink-commits@lists.sourceforge.net/msg113511.html>

set(PACKAGE itk)

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
# Drop -fvisibility=hidden since that option does not work for itk.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

set(VERSION 4.0.0)
set(DIRVERSION 4.0.0)
if(MSYS_PLATFORM)
  string(REGEX REPLACE "\\." "" LIBVERSION ${DIRVERSION})
else(MSYS_PLATFORM)
  set(LIBVERSION ${DIRVERSION})
endif(MSYS_PLATFORM)

# Data that is related to downloads.
set(URL http://downloads.sourceforge.net/project/incrtcl/%5bincr%20Tcl_Tk%5d-4-source/Itcl%20${VERSION}/itk${VERSION}.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH e3600a9ad0fcdcbbc4138af5a4893b7e)

if(EPA_HAVE_64_BIT_OS)
  set(ENABLE_64_BIT --enable-64bit)
endif(EPA_HAVE_64_BIT_OS)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  PATCH_COMMAND ""
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" "CPPFLAGS=-I${EPA_CMAKE_INSTALL_PREFIX}/include" ${source_PATH}/${EPA_CONFIGURE_COMMAND} --mandir=${EPA_CMAKE_INSTALL_PREFIX}/share/man ${ENABLE_64_BIT} --with-itcl=${EPA_CMAKE_INSTALL_PREFIX}/lib/itcl${DIRVERSION}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-patch
  COMMAND ${CMAKE_COMMAND} -E echo
  "Provide an exactly equivalent \"Itk\" and \"itk\" form of the package name to follow what is implemented for itcl version 4."
  # This patch has been publicly distributed at https://sourceforge.net/p/incrtcl/patches/52/
  COMMAND ${PATCH_EXECUTABLE} --directory=${EPA_BASE}/Source/build_${PACKAGE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/itk4_case.patch
  COMMAND ${CMAKE_COMMAND} -E echo
  "Add generic/itkDecls.h to the list of itk headers that must be installed."
  # This patch has been publicly distributed at https://sourceforge.net/p/incrtcl/patches/53/
  COMMAND ${PATCH_EXECUTABLE} --directory=${EPA_BASE}/Source/build_${PACKAGE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/itk4_header_list.patch
  COMMAND ${CMAKE_COMMAND} -E echo
  "Apply a patch for the results of \"autoreconf -i\" run on Linux."
  COMMAND ${PATCH_EXECUTABLE} --directory=${EPA_BASE}/Source/build_${PACKAGE} -p1 < ${CMAKE_CURRENT_SOURCE_DIR}/autoreconf.patch
  APPEND
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-build
  COMMAND echo "Replace build-tree locations by install-tree locations"
  COMMAND ${SED_EXECUTABLE}
  -e "s@^\\(ITK_SRC_DIR='\\).*@\\1${EPA_CMAKE_INSTALL_PREFIX}/include'@"
  # Comment out the next since itk build system does not configure any
  # of these variables so they are left in symbolic "@" form.
  #-e "/ITK_B/s@='\\(-L\\)\\?.*build_itk@='\\1${EPA_CMAKE_INSTALL_PREFIX}/lib@"
  -i itkConfig.sh
  APPEND
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND echo "Install-tree fixups"
  COMMAND ${CHMOD_EXECUTABLE} -v ${SO_NUMERICAL_PERMISSIONS} ${EPA_CMAKE_INSTALL_PREFIX}/lib/itk${DIRVERSION}/${library_prefix}itk${LIBVERSION}${library_suffix}
  APPEND
  )
