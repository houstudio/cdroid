# freetype_nohb/CMakeLists.txt

# Configure the build of freetype_nohb.

# To follow what is recommended by BLFS, the freetype package (which
# is identical to the current package except for the --with-harfbuzz
# configure option) depends on harfbuzz, which in turn depends on the
# current package which because it uses the --without-harfbuzz
# configure option depends on no other package.

# Copyright (C) 2014 Alan W. Irwin

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

set(PACKAGE freetype_nohb)

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
# Drop -fvisibility=hidden since that option does not work for this package.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")

# Data that is related to downloads.
set(URL http://downloads.sourceforge.net/freetype/freetype-2.5.3.tar.bz2)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH d6b60f06bfc046e43ab2a6cbfd171d65)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND} --without-harfbuzz  --disable-static
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  BUILD_IN_SOURCE 0
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH}  ${EPA_PARALLEL_MAKE_COMMAND} install
  )

# Note BLFS recommends the extra steps of installing the
# documentation, enabling GX/AAT and OpenType table validation, and
# enabling Subpixel Rendering and Subpixel Hinting in order to improve
# font rendering.  However, for simplicity we do none of those extra
# steps for now (which also renders moot the possibility [discussed in
# detail by BLFS] that Subpixel Rendering _might_ be patent
# encumbered).
