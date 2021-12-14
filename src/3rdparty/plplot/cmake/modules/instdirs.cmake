# cmake/modules/instdirs.cmake
#
# Copyright (C) 2006-2017  Alan W. Irwin
#
# This file is part of PLplot.
#
# PLplot is free software; you can redistribute it and/or modify
# it under the terms of the GNU Library General Public License as published
# by the Free Software Foundation; version 2 of the License.
#
# PLplot is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU Library General Public License for more details.
#
# You should have received a copy of the GNU Library General Public License
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Module for determining all installation directories for PLplot.

# Note, use absolute install locations only since relative ones are
# automatically prefixed with ${CMAKE_INSTALL_PREFIX} which may destroy
# the fine-tuning of installation locations that we desire.

# Cached install locations following what is done for the autotools configure
# script: (CMAKE_INSTALL_SBINDIR, CMAKE_INSTALL_LIBEXECDIR,
# CMAKE_INSTALL_SHAREDSTATEDIR, CMAKE_INSTALL_LOCALSTATEDIR, and
# CMAKE_INSTALL_OLDINCLUDEDIR not set because PLplot does not use those
# install locations).

# N.B. This autotools-like set of variables not directly used for
# installations, but they are used to determine default install
# locations below (without the CMAKE_INSTALL prefix) that are directly
# used for installations.

set(INSTALL_LOCATION_VARIABLES_LIST)
set(
  CMAKE_INSTALL_EXEC_PREFIX
  ${CMAKE_INSTALL_PREFIX}
  CACHE PATH "General install location for architecture-dependent files"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_EXEC_PREFIX)

set(
  CMAKE_INSTALL_BINDIR
  ${CMAKE_INSTALL_EXEC_PREFIX}/bin
  CACHE PATH "General install location for user executables"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_BINDIR)

set(
  CMAKE_INSTALL_DATADIR
  ${CMAKE_INSTALL_PREFIX}/share
  CACHE PATH "General install location for read-only architecture-independent data"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_DATADIR)

set(
  CMAKE_INSTALL_LIBDIR
  ${CMAKE_INSTALL_EXEC_PREFIX}/lib
  CACHE PATH "General install location for object code libraries"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_LIBDIR)

set(
  CMAKE_INSTALL_INCLUDEDIR
  ${CMAKE_INSTALL_PREFIX}/include
  CACHE PATH "General install location for C header files"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_INCLUDEDIR)

set(
  CMAKE_INSTALL_INFODIR
  ${CMAKE_INSTALL_DATADIR}/info
  CACHE PATH "General install location for info documentation"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_INFODIR)

set(
  CMAKE_INSTALL_MANDIR
  ${CMAKE_INSTALL_DATADIR}/man
  CACHE PATH "General install location for man documentation"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_MANDIR)

set(
  CMAKE_INSTALL_PKG_CONFIG_DIR
  ${CMAKE_INSTALL_LIBDIR}/pkgconfig
  CACHE PATH "General install location for pkg-config *.pc files"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST CMAKE_INSTALL_PKG_CONFIG_DIR)

# Configured PLplot install locations with default values
# determined from the more general user-updatable cached values above.
# These are the actual install locations used by PLplot.
# Make these values cached as well so the user
# can change from these defaults if necessary.

set(
  DATA_DIR
  ${CMAKE_INSTALL_DATADIR}/${PACKAGE}${PLPLOT_VERSION}
  CACHE PATH "PLplot install location for architecture-independent data"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST DATA_DIR)

set(
  LIB_DIR
  ${CMAKE_INSTALL_LIBDIR}
  CACHE PATH "PLplot install location for object code libraries"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST LIB_DIR)

set(
  INCLUDE_DIR
  ${CMAKE_INSTALL_INCLUDEDIR}/${PACKAGE}
  CACHE PATH "PLplot install location for C header files"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST INCLUDE_DIR)

set(
  BIN_DIR
  ${CMAKE_INSTALL_BINDIR}
  CACHE PATH "PLplot install location for user executables"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST BIN_DIR)

set(
  DRV_DIR
  ${CMAKE_INSTALL_LIBDIR}/${PACKAGE}${PLPLOT_VERSION}/drivers
  CACHE PATH "PLplot install location for dynamically loaded devices"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST DRV_DIR)

set(
  DOC_DIR
  ${CMAKE_INSTALL_DATADIR}/doc/${PACKAGE}
  CACHE PATH "PLplot install location for (architecture-independent) documentation files"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST DOC_DIR)

set(
  INFO_DIR
  ${CMAKE_INSTALL_INFODIR}
  CACHE PATH "PLplot install location for (architecture-independent) info files"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST INFO_DIR)

set(
  MAN_DIR
  ${CMAKE_INSTALL_MANDIR}
  CACHE PATH "PLplot install location for (architecture-independent) man files"
  )
list(APPEND INSTALL_LOCATION_VARIABLES_LIST MAN_DIR)

# End of fundamental user-settable install locations here, but
# additional cached install locations are added later (with
# appropriate append of the relevant variable name to
# INSTALL_LOCATION_VARIABLES_LIST) depending on what capabilities of
# PLplot are enabled.

# Other path-related variables.

# Absolute path of top-level build directory.
set(BUILD_DIR ${CMAKE_BINARY_DIR})

# Absolute path of top-level source directory.
set(SOURCE_DIR ${CMAKE_SOURCE_DIR})

# The following *_CMD variables are needed in
# examples/*/Makefile.examples.cmake

set(LIB_DIR_CMD "-L${LIB_DIR}")
set(INCLUDE_DIR_CMD "-I${INCLUDE_DIR}")
