# qt5_lite/CMakeLists.txt

# Configure the build of qt5_lite.

# Copyright (C) 2014-2015 Alan W. Irwin

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
# <http://www.linuxfromscratch.org/blfs/view/svn/x/qt5.html>.

set(PACKAGE qt5_lite)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

# The assumption here is that for Windows platforms Qt5 is self-contained
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

# Drop -fvisibility=hidden since that option may not work for Qt5.
# (May test this later if I get everything to work.)
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")
string(REGEX REPLACE "-fvisibility=hidden" "" CXXFLAGS "${CXXFLAGS}")

# Data that is related to downloads.
# set(URL http://download.qt-project.org/official_releases/qt/5.2/5.2.1/single/qt-everywhere-opensource-src-5.2.1.tar.xz)
# N.B. tar.gz used because that is the one advertised by the Qt5 download site rather than the tar.xz
# version that is also available (but unadvertised).  The .gz version is 34 per cent larger which
# implies the download will typically take ~1 minute longer, but the unpack should be much faster
# so it is not a clear choice.
#set(URL http://download.qt-project.org/official_releases/qt/5.3/5.3.1/single/qt-everywhere-opensource-src-5.3.1.tar.gz)
set(URL http://download.qt-project.org/official_releases/qt/5.3/5.3.2/single/qt-everywhere-opensource-src-5.3.2.tar.gz)
# 5.4.1 would not build with error: operator '||' has no right operand and many subsequent
# error: <component> "is not a template" errors.  The configuration used was
# identical to to 5.3.x, and changing that may allow dropping components of
# the build with such build issues.
#set(URL http://download.qt-project.org/official_releases/qt/5.4/5.4.1/single/qt-everywhere-opensource-src-5.4.1.tar.gz)
# Temporary to save download bandwidth/time while debugging this.
# set(URL /home/software/build_qt5/qt-everywhere-opensource-src-5.2.1.tar.xz)
# set(URL /home/software/build_qt5/qt-everywhere-opensource-src-5.3.1.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
# 5.2.1
#set(DOWNLOAD_HASH 0c8d2aa45f38be9c3f7c9325eb059d9d)
#set(DOWNLOAD_HASH f9a24a0d5645efa0715b6ff0fa13d60f)
# 5.3.1 (from http://download.qt-project.org/official_releases/qt/5.3/5.3.1/single/md5sums.txt).
# set(DOWNLOAD_HASH a355749e4f200a6392ecad151d1d0cdc)
# 5.3.2 (from http://download.qt-project.org/official_releases/qt/5.3/5.3.2/single/md5sums.txt).
set(DOWNLOAD_HASH febb001129927a70174467ecb508a682)
# 5.4.1 (from http://download.qt-project.org/official_releases/qt/5.4/5.4.1/single/md5sums.txt).
#set(DOWNLOAD_HASH 90f3fbce38ed16e5dc2cd0909ae86ca4)

# Many of these are recommended by BLFS
# <http://www.linuxfromscratch.org/blfs/view/svn/x/qt5.html>, but I
# have tried to use a consistent install prefix and I have also turned
# off as much as possible for this "lite" version.
set(config_arguments

  # Configure options:
  -release
  -confirm-license
  -opensource

  # Try to get rid of everything to do with sql by eliminating all possible
  # drivers.  5.2.1 list = 5.3.1 list = 5.3.2 list =
  # db2 ibase mysql oci odbc psql sqlite sqlite2 tds
  -no-sql-db2
  -no-sql-ibase
  -no-sql-mysql
  -no-sql-oci
  -no-sql-odbc
  -no-sql-psql
  -no-sql-sqlite
  -no-sql-sqlite2
  -no-sql-tds

  # Comment out for 5.3.1 and 5.3.2 since (-no-)javascript-jit no longer an option
  # -no-javascript-jit
  -no-qml-debug
  # Third party libraries (drop if possible but if needed by PLplot, use
  # in-house Qt5 versions of these to reduce dependencies):
  -qt-zlib
  -no-mtdev
  -no-gif
  -qt-libpng
  -qt-libjpeg
  -qt-freetype
  # Experimental in-house harfbuzz NG (5.2.1 and 5.3.1)
  # Comment out for now ==> no harfbuzz NG.
  #-qt-harfbuzz
  -no-openssl
  -qt-pcre
  -qt-xcb
  -qt-xkbcommon
  -no-xinput2
  -no-xcb-xlib
  -no-glib
  # These 3 external library options available with 5.3.[1-2] only
  -no-pulseaudio
  -no-alsa
  -no-gtkstyle

  # Additional options:

  # Drop all components other than libs
  -nomake tools
  -nomake examples
  -nomake tests

  # These skips have so far only been tried for 5.3.1 and 5.3.2
  # The list is taken from the top-level subdirectories in
  # the 5.3.1 and 5.3.2 source trees that start with "qt".
  # (It turns out this list of directories is the same for both
  # 5.3.1 and 5.3.2.)  I also
  # consulted <http://doc.qt.io/qt-5/qtmodules.html>
  # for explanation of the purpose of these various modules.

  -skip qtactiveqt
  -skip qtandroidextras
  # Don't skip qtbase for obvious reasons.
  # -skip qtbase
  -skip qtconnectivity
  -skip qtdeclarative
  -skip qtdoc
  -skip qtenginio
  -skip qtgraphicaleffects
  # Don't skip qtimageformats which supports TIFF, MNG, TGA, WBMP.
  # -skip qtimageformats
  -skip qtlocation
  # Don't skip qtmacextras which provides platform-specific APIs for Mac OS X.
  #-skip qtmacextras
  -skip qtmultimedia
  -skip qtquick1
  -skip qtquickcontrols
  -skip qtscript
  -skip qtsensors
  -skip qtserialport
  # Don't skip qtsvg module which contains the QSvgGenerator class used to create
  # SVG file results.
  # -skip qtsvg
  -skip qttools
  -skip qttranslations
  -skip qtwebkit
  -skip qtwebkit-examples
  -skip qtwebsockets
  -skip qtwinextras
  # Don't skip qtx11extras which provides platform-specific APIs for X11.
  # -skip qtx11extras
  -skip qtxmlpatterns

  # Speed compilation of qmake which presumably does not have to
  # be that efficient.
  -no-optimized-qmake

  -no-compile-examples
  # Need gui
  #-no-gui
  # Need widgets for some reason to have PrintSupport
  #-no-widgets
  -no-nis
  -no-cups
  -no-iconv
  -no-evdev
  -no-icu
  -no-dbus
  -no-eglfs
  -no-directfb
  -no-linuxfb
  -no-kms

  # There are a whole bunch of features listed in
  # qtbase/src/corelib/global/qfeatures.txt which
  # can apparently be dropped using
  # -no-feature-<feature>
  # (where I assume <feature> is the uppercased macro name
  # of the feature mentioned in qfeatures.txt), but
  # this is such fine-grained control without much
  # documentation that I am going to use default
  # features and not try these -no-feature options.

  -no-opengl
  # Qt/X11 only: (leave everything default)

  )

# Note, setting the QT5DIR environment variable to the install prefix
# as below is recommended by method 2 of BLFS.
set(QT5DIR ${EPA_CMAKE_INSTALL_PREFIX})

ExternalProject_Add(
  build_qt5_lite
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} QT5DIR=${QT5DIR} "CFLAGS=${CFLAGS}" "CXXFLAGS=${CXXFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND} ${config_arguments}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} QT5DIR=${QT5DIR} ${EPA_PARALLEL_MAKE_COMMAND}
  BUILD_IN_SOURCE 0
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} QT5DIR=${QT5DIR} ${EPA_PARALLEL_MAKE_COMMAND} install
  STEP_TARGETS update configure build install test
  )

file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install_png.sh
"install -v -dm755 ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/qttools/src/assistant/assistant/images/assistant-128.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/assistant-qt5.png
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/qttools/src/designer/src/designer/images/designer.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/designer-qt5.png
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/qttools/src/linguist/linguist/images/icons/linguist-128-32.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/linguist-qt5.png
install -v -Dm644 ${EPA_BASE}/Source/build_${PACKAGE}/qttools/src/qdbus/qdbusviewer/images/qdbusviewer-128.png ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/pixmaps/qdbusviewer-qt5.png
install -dm755 ${EPA_CMAKE_INSTALL_PREFIX}/usr/share/applications
"
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND ${CMAKE_COMMAND} -E echo "install some additional png files recommended by BLFS."
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/install_png.sh
  APPEND
  )

# 5.2.1 and 5.3.1 version the same
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/install_fixups.sh
"find ${QT5DIR} -name qt_lib_bootstrap_private.pri -exec sed -i -e \"s:$PWD/qtbase:/${QT5DIR}/lib/:g\" {} \;
find ${QT5DIR} -name \\*.prl -exec sed -i -e '/^QMAKE_PRL_BUILD_DIR/d' {} \;
"
  )

add_custom_command(
  OUTPUT
  ${EPA_BASE}/Stamp/build_${PACKAGE}/build_${PACKAGE}-install
  COMMAND ${CMAKE_COMMAND} -E echo "install fixups recommended by BLFS."
  COMMAND ${BASH_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/install_fixups.sh
  APPEND
  )

# N.B. I have explicitly ignored all post-install Qt5 configuration recommended
# by BLFS because I don't think that will be needed by PLplot.
