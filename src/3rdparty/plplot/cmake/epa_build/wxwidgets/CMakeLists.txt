# wxwidgets/CMakeLists.txt

# Configure the build of wxwidgets.

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

set(PACKAGE wxwidgets)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

if(MSYS_PLATFORM)
  # No dependencies required on Windows.
  set(dependencies_LIST)
else(MSYS_PLATFORM)
  # To avoid having to build all of gtk with epa_build in a consistent
  # manner I tried several alternatives to the gtk toolkit.
  # e.g.,
  # set(toolkit_options --with-x11)
  # set(toolkit_options --with-motif)
  # But neither of those worked because they are not capable of
  # supplying the unicode (see --enable-unicode below) capability
  # that is provided by the default gtk toolkit and needed by the
  # PLplot build of the wxwidgets device.
  # The inevitable conclusion is that on Unix, we must use the gtk toolkit
  # and therefore (now that epa_build has a gtk+ configuration)
  # we must specificially add gtk+ as a dependency of wxwidgets.
  set(dependencies_LIST gtk+)
endif(MSYS_PLATFORM)

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
  set(CXXFLAGS "-fno-keep-inline-dllexport $ENV{CXXFLAGS}")
else(MSYS_PLATFORM)
  set(CXXFLAGS "$ENV{CXXFLAGS}")
  # --with-gtk=3 because of gtk+ (version 3) dependency above
  set(toolkit_options --enable-debug --enable-debug_gdb --with-gtk=3)
endif(MSYS_PLATFORM)

set(CFLAGS "$ENV{CFLAGS}")

# Drop -fvisibility=hidden since that option may not work for this package.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")
string(REGEX REPLACE "-fvisibility=hidden" "" CXXFLAGS "${CXXFLAGS}")

# Data that is related to downloads.
# N.B. 2.8.12 does not work properly with gtk+ version 3.  At minimum
# you need wxwidgets-2.9.4 (which is the first version to honor the
# ---with-gtk=3 option below).
#set(URL http://prdownloads.sourceforge.net/wxwindows/wxWidgets-2.8.12.tar.gz)
#set(URL_HASH MD5=2fa39da14bc06ea86fe902579fedc5b1)
#set(URL http://prdownloads.sourceforge.net/wxwindows/3.0.0/wxWidgets-3.0.0.tar.bz2)
#set(URL_HASH SHA1=756a9c54d1f411e262f03bacb78ccef085a9880a)
set(URL http://prdownloads.sourceforge.net/wxwindows/3.0.2/wxWidgets-3.0.2.tar.bz2)
set(URL_HASH SHA1=6461eab4428c0a8b9e41781b8787510484dea800)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS "${dependencies_targets}"
  URL ${URL}
  URL_HASH ${URL_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" "CXXFLAGS=${CXXFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND} --enable-shared --enable-unicode ${toolkit_options}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  BUILD_IN_SOURCE 1
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install
  )
