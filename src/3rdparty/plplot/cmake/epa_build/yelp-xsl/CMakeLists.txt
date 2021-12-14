# yelp-xsl/CMakeLists.txt

# Configure the build of yelp-xsl.

# N.B. this file is generated so if you edit it you will lose all your
# changes the next time it is generated (typically by running
# either/both the update_added_packages.sh or update_pango_packages.sh
# scripts).  If those scripts do not provide good results, then
# consider changing their source files (e.g., by editing the files
# used in those scripts) or add a patch to be run by those scripts.

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

set(PACKAGE yelp-xsl)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST pkg-config;bison;flex;python2-devel;libX11;libXft;xorg-kbproto;xorg-macros;xorg-xproto;dbus ${extra_ignored_dependencies_list})

set(dependencies_LIST intltool;itstool;libxml2;libxslt)

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
  set(CFLAGS "$ENV{CFLAGS}")
else(MSYS_PLATFORM)
  set(CFLAGS "$ENV{CFLAGS}")
endif(MSYS_PLATFORM)

set(CXXFLAGS "$ENV{CXXFLAGS}")

# Drop -fvisibility=hidden since that option does not work for a
# number of software packages that are configured automatically using
# this template.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")
string(REGEX REPLACE "-fvisibility=hidden" "" CXXFLAGS "${CXXFLAGS}")

# Data that is related to downloads.
set(URL http://download.gnome.org/sources/yelp-xsl/3.8/yelp-xsl-3.8.1.tar.xz)
set(DOWNLOAD_HASH_TYPE SHA256)
set(DOWNLOAD_HASH afc519be07ffcbdcc20d1acfd1d901cfd03c7b4cf0667036534f2726bb68b40d)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" "CXXFLAGS=${CXXFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  BUILD_IN_SOURCE OFF
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}  install
  )
