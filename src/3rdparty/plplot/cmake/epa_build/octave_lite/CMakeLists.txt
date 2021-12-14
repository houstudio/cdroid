# octave_lite/CMakeLists.txt

# Configure the build of octave_lite.

# Copyright (C) 2015 Alan W. Irwin

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

set(PACKAGE octave_lite)

# List of dependencies (most of which are build tools) which should be
# ignored.
set(ignored_dependencies_LIST ${extra_ignored_dependencies_list})

# libpcre is built (as a dependency of swig) and lapack_blas is built
# when BUILD_THE_BUILDTOOLS is ON so do not express these dependencies
# of octave-lite here.
set(dependencies_LIST libqhull)

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
set(FFLAGS "$ENV{FFLAGS}")

# Drop -fvisibility=hidden since that option does not work for a
# number of software packages that are configured automatically using
# this template.
string(REGEX REPLACE "-fvisibility=hidden" "" CFLAGS "${CFLAGS}")
string(REGEX REPLACE "-fvisibility=hidden" "" CXXFLAGS "${CXXFLAGS}")

# Data that is related to downloads.
# Simple tests of octave bindings for PLplot for epa_built octave_lite 4.0.0 fail
# so try 3.8.2 version instead.
#set(URL ftp://ftp.gnu.org/gnu/octave/octave-4.0.0.tar.xz)
set(URL ftp://ftp.gnu.org/gnu/octave/octave-3.8.2.tar.gz)
set(DOWNLOAD_HASH_TYPE MD5)
#set(DOWNLOAD_HASH f3de0a0d9758e112f13ce1f5eaf791bf)
set(DOWNLOAD_HASH ed6ab54e5259a6e1ca7ece192026745a)

# Find full pathname of blas and lapack libraries.
# This only works if Fortran is enabled.
enable_language(Fortran)
# Required to obtain blas that we have epa_built.
set(BLA_VENDOR Generic)
find_package(LAPACK REQUIRED)
message(STATUS "LAPACK_LIBRARIES = ${LAPACK_LIBRARIES}")

set(LIB_BLAS)
set(LIB_LAPACK)
foreach(library IN LISTS LAPACK_LIBRARIES)
  message(STATUS "library = ${library}")
  if(library MATCHES ".*blas.*")
    set(LIB_BLAS ${library})
  elseif(library MATCHES ".*lapack.*")
    set(LIB_LAPACK ${library})
  endif(library MATCHES ".*blas.*")
  if(LIB_BLAS AND LIB_LAPACK)
       break()
  endif(LIB_BLAS AND LIB_LAPACK)
endforeach(library IN LISTS LAPACK_LIBRARIES)

message(STATUS "LIB_BLAS = ${LIB_BLAS}")
message(STATUS "LIB_LAPACK = ${LIB_LAPACK}")

if(NOT (LIB_BLAS AND LIB_LAPACK))
  message(FATAL_ERROR "Could not find blas and lapack libraries necessary for epa_build of octave_lite")
endif(NOT (LIB_BLAS AND LIB_LAPACK))

get_filename_component(LIB_BLAS ${LIB_BLAS} PATH)
set(LIB_BLAS "-L${LIB_BLAS} -lblas")
get_filename_component(LIB_LAPACK ${LIB_LAPACK} PATH)
set(LIB_LAPACK "-L${LIB_LAPACK} -llapack")

message(STATUS "(processed) LIB_BLAS = ${LIB_BLAS}")
message(STATUS "(processed) LIB_LAPACK = ${LIB_LAPACK}")

set(configure_options
  --disable-silent-rules
  --disable-java
  --with-qhull-includedir=${EPA_CMAKE_INSTALL_PREFIX}/include
  --with-qhull-libdir=${EPA_CMAKE_INSTALL_PREFIX}/lib
  --without-z
  --without-hdf5
# Works properly (no fftw build errors) only with 4.0.0
#  --without-fftw3
#  --without-fftw3f
  --without-glpk
  --without-curl
#4.0.0  --without-sndfile
#4.0.0  --without-portaudio
  --without-framework-carbon
  --without-opengl
  --without-framework-opengl
#4.0.0  --without-fltk
#4.0.0  --without-OSMesa
  --with-blas=${LIB_BLAS}
  --with-lapack=${LIB_LAPACK}
  --without-qrupdate
  --without-amd
  --without-camd
  --without-colamd
  --without-ccolamd
  --without-cholmod
  --without-cxsparse
  --without-umfpack
  --without-arpack
)

ExternalProject_Add(
  build_${PACKAGE}
  DEPENDS ${dependencies_targets}
  URL ${URL}
  URL_HASH ${DOWNLOAD_HASH_TYPE}=${DOWNLOAD_HASH}
  CONFIGURE_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} "CFLAGS=${CFLAGS}" "CXXFLAGS=${CXXFLAGS}" "FFLAGS=${FFLAGS}" ${source_PATH}/${EPA_CONFIGURE_COMMAND} ${configure_options}
  BUILD_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND}
  INSTALL_COMMAND ${ENV_EXECUTABLE} PATH=${EPA_PATH} ${EPA_PARALLEL_MAKE_COMMAND} install
  )
