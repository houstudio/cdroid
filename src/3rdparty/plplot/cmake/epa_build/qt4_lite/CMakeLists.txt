# qt4_lite/CMakeLists.txt

# Configure the build of qt4_lite.

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

# Many of the details of this epa_build configuration were derived from
# <http://www.linuxfromscratch.org/blfs/view/svn/x/qt4.html>.

set(PACKAGE qt4_lite)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

# The assumption here is that for Windows platforms Qt4 is self-contained
# and therefore does not rely on external libraries (at least if you configure
# the in-house version of those).  For Linux platforms the dependencies will
# just have to be resolved by system versions rather than building a lot
# of additional packages.
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
set(CXXFLAGS "$ENV{CXXFLAGS}")

# Drop -fvisibility=hidden since that option may not work for Qt4.
# (May test this later if I get everything to work.)
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")
string(REGEX REPLACE "-fvisibility=hidden" "" CXXFLAGS "${CXXFLAGS}")

# Data that is related to downloads.
set(URL http://download.qt-project.org/official_releases/qt/4.8/4.8.5/qt-everywhere-opensource-src-4.8.5.tar.gz)
# Temporary to save download bandwidth/time while debugging this.
#set(URL /home/software/build_qt4/qt-everywhere-opensource-src-4.8.5.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
set(DOWNLOAD_HASH 1864987bdbb2f58f8ae8b350dfdbe133)

# Many of these are recommended by BLFS, but I have tried to use
# a consistent install prefix and I have also turned off as
# much as possible for this "lite" version.
set(config_arguments

  # Installation options:
  -sysconfdir ${EPA_CMAKE_INSTALL_PREFIX}/etc/xdg

  # Configure options:
  -release
  -confirm-license
  -opensource
  # Need to be consistent with system versions of PyQt4/QtGui.so which need
  # a libQtGui.so library which contains QWidget::accessibleName() const
  # and QWidget::accessibleDescription() const.
  -accessibility

  # Try to get rid of everything to do with sql by eliminating all possible
  # drivers.
  -no-sql-db2
  -no-sql-ibase
  -no-sql-mysql
  -no-sql-oci
  -no-sql-odbc
  -no-sql-psql
  -no-sql-sqlite
  -no-sql-sqlite2
  -no-sql-sqlite_symbian
  -no-sql-symsql
  -no-sql-tds

  -no-qt3support
  -no-xmlpatterns
  -no-multimedia
  -no-audio-backend
  -no-phonon
  -no-phonon-backend

  # Need svg
   -svg

  -no-webkit
  -no-javascript-jit
  -no-script
  -no-scripttools
  -no-declarative
  -no-declarative-debug
  # Third party libraries (drop if possible but if needed by PLplot, use
  # in-house Qt4 versions of these to reduce dependencies):
  -qt-zlib
  -no-gif
  -qt-libtiff
  -qt-libpng
  -no-libmng
  -qt-libjpeg
  -no-openssl
  # Additional options:

  # Drop all components other than libs
  -nomake tools
  -nomake examples
  -nomake demos
  -nomake docs
  -nomake translations

  -optimized-qmake

  # Need gui which happens by default (there is no -gui option)
  # if you do not specify -no-gui

  -no-nis
  -no-cups
  -no-iconv
  -no-dbus

  # Qt/X11 only: (leave everything default)
  )

# Note QTDIR= below.  Setting that environment variable is recommended by method 2 of BLFS.

ExternalProject_Add(
  build_qt4_lite
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} QTDIR=${EPA_CMAKE_INSTALL_PREFIX} "CFLAGS=${CFLAGS}" "CXXFLAGS=${CXXFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND} ${config_arguments}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} QTDIR=${EPA_CMAKE_INSTALL_PREFIX} ${EPA_PARALLEL_MAKE_COMMAND}
  BUILD_IN_SOURCE 0
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} QTDIR=${EPA_CMAKE_INSTALL_PREFIX} ${EPA_PARALLEL_MAKE_COMMAND} install
  #STEP_TARGETS update configure build install test
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-update
  COMMAND ${CMAKE_COMMAND} -E echo "make two code fixups recommended by BLFS."
  COMMAND ${SED_EXECUTABLE} -i -e "/#if/d" -e "/#error/d" -e "/#endif/d" ${EPA_BASE}/Source/build_${PACKAGE}/config.tests/unix/libmng/libmng.cpp
  COMMAND ${SED_EXECUTABLE} -i "/CONFIG -/ a\\isEmpty(OUTPUT_DIR): OUTPUT_DIR = ../.." ${EPA_BASE}/Source/build_${PACKAGE}/src/3rdparty/webkit/Source/WebKit2/DerivedSources.pro
  APPEND
  )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install_png.sh
"install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/src/gui/dialogs/images/qtlogo-64.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/qt4logo.png
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/tools/assistant/tools/assistant/images/assistant-128.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/assistant-qt4.png
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/tools/designer/src/designer/images/designer.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/designer-qt4.png
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/tools/linguist/linguist/images/icons/linguist-128-32.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/linguist-qt4.png
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/tools/qdbus/qdbusviewer/images/qdbusviewer-128.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/qdbusviewer-qt4.png
"
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND ${CMAKE_COMMAND} -E echo "install some addition png files recommended by BLFS."
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/install_png.sh
  APPEND
  )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install_fixups.sh
"for file in `basename -a -s .prl ${EPA_CMAKE_INSTALL_PREFIX}/lib/lib*.prl`; do
  sed -i -e '/^QMAKE_PRL_BUILD_DIR/d;s/\(QMAKE_PRL_LIBS =\).*/\\1/' ${EPA_CMAKE_INSTALL_PREFIX}/lib/\${file}.prl
  perl -pi -e \"s, -L\$PWD/?\\S+,,g\" ${EPA_CMAKE_INSTALL_PREFIX}/lib/pkgconfig/\${file##lib}.pc
done

unset file
"
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND ${CMAKE_COMMAND} -E echo "install fixups recommended by BLFS."
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/install_fixups.sh
  APPEND
  )

# N.B. I have explicitly ignored all post-install Qt4 configuration recommended
# by BLFS because I don't think that will be needed by PLplot.
