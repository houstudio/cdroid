# cmake/modules/plplot.cmake
#
# Copyright (C) 2006, 2007, 2008, 2009 Alan W. Irwin
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

# Module file to decide whether to build doxygen documentation and
# and, if so, find the necessary doxygen command.  N.B. turn OFF by
# default since the doxygen build now requires something like 10
# minutes (because of the call graph and caller graph generation), and
# that build latency would be annoying for developers who are just
# making some trivial change in the source tree.

option(BUILD_DOX_DOC "Build doxygen documentation" OFF)

option(PREBUILT_DOX_DOC "Assume doxygen documentation is already built and present in doc/doxygen, ready to be installed. This option is useful for package maintainers" OFF)

if(BUILD_DOX_DOC AND PREBUILT_DOX_DOC)
  message(FATAL_ERROR "Options BUILD_DOX_DOC and PREBUILT_DOX_DOC are logically exclusive and must not be set simultaneously. Giving up.")
endif(BUILD_DOX_DOC AND PREBUILT_DOX_DOC)

if(BUILD_DOX_DOC)
  find_package(Doxygen)
  if(NOT DOXYGEN_FOUND)
    message(STATUS "WARNING: "
      "Cannot find doxygen so disabling doxygen documentation build.")
    set(BUILD_DOX_DOC OFF CACHE BOOL "Build doxygen documentation" FORCE)
  endif(NOT DOXYGEN_FOUND)
endif(BUILD_DOX_DOC)
