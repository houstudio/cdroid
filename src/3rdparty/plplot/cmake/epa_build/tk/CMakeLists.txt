# tk/CMakeLists.txt

# Configure the build of tk.

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

# These build configuration details for Tk adapted from information in
# <http://www.linuxfromscratch.org/blfs/view/svn/general/tk.html>.

set(PACKAGE tk)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

set(dependencies_LIST tcl)

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

# Data that is related to the PATH that must be used.
# N.B. note below that we currently use the unix subdirectory for the
# Mac OS X case.  There is also a Tcl macosx subdirectory we might
# want to try in that case once an epa_build user with access to a Mac
# OS X platform gives this a try.
if(MSYS_PLATFORM)
  set(source_PATH ${source_PATH}/win)
  set(executable_suffix .exe)
  set(library_suffix .a)
  # Workaround for the Tk autotools build system for Windows not
  # propagating CFLAGS to windres.
  set(make_args "RC=windres -I${EPA_CMAKE_INSTALL_PREFIX}/include")
else(MSYS_PLATFORM)
  set(source_PATH ${source_PATH}/unix)
  set(executable_suffix)
  set(library_suffix .so)
  set(make_args)
endif(MSYS_PLATFORM)

set(CFLAGS "$ENV{CFLAGS}")
# Drop -fvisibility=hidden since that option may not work for this package.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

set(TCL_TK_VERSION 8.6.1)
string(REGEX REPLACE "\\.[0-9]$" "" TCL_TK_LIBVERSION ${TCL_TK_VERSION})
if(MSYS_PLATFORM)
  string(REGEX REPLACE "\\." "" TCL_TK_LIBVERSION ${TCL_TK_LIBVERSION})
endif(MSYS_PLATFORM)

# Data that is related to downloads.
set(URL http://downloads.sourceforge.net/project/tcl/Tcl/${TCL_TK_VERSION}/tk${TCL_TK_VERSION}-src.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH 63f21c3a0e0cefbd854b4eb29b129ac6)

if(EPA_HAVE_64_BIT_OS)
  set(ENABLE_64_BIT --enable-64bit)
endif(EPA_HAVE_64_BIT_OS)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS} -I${EPA_CMAKE_INSTALL_PREFIX}/include" ${source_PATH}/${EPA_CONFIGURE_COMMAND} --mandir=${EPA_CMAKE_INSTALL_PREFIX}/share/man ${ENABLE_64_BIT}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} ${make_args}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install ${make_args}
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-build
  COMMAND echo "Replace build-tree locations by install-tree locations"
  COMMAND ${SED_EXECUTABLE}
  -e "s@^\\(TK_SRC_DIR='\\).*@\\1${EPA_CMAKE_INSTALL_PREFIX}/include'@"
  -e "/TK_B/s@='\\(-L\\)\\?.*build_tk@='\\1${EPA_CMAKE_INSTALL_PREFIX}/lib@"
  -i tkConfig.sh
  APPEND
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND echo "Install-tree fixups"
  COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install-private-headers
  COMMAND ${LN_EXECUTABLE} -v -sf wish${TCL_TK_LIBVERSION}${executable_suffix} ${EPA_CMAKE_INSTALL_PREFIX}/bin/wish${executable_suffix}
  COMMAND ${CHMOD_EXECUTABLE} -v ${SO_NUMERICAL_PERMISSIONS} ${EPA_CMAKE_INSTALL_PREFIX}/lib/libtk${TCL_TK_LIBVERSION}${library_suffix}
  APPEND
  )
