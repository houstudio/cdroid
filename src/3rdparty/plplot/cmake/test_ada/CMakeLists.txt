# Top-level CMakeLists.txt for a simple project consisting of a
# "Hello, world" library and executable to test CMake language support
# for Ada. See the README file in this directory for more details.

# MAINTENANCE
# Use same minimum version for all platforms as the Linux platform minimum adopted for
# the PLplot project.
cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)

# Before the project command must specify the location where
# the test_ada project looks for CMake Ada language support files.
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/Modules")

# Add C to list of languages enabled to work around some issues with
# CMake language support for Ada.  CMake uses compiler introspection
# to determine some system information such as the principal library
# location for the system, and that part of Ada language support is
# not implemented.  Also, I have discovered one other issue (probably
# related to compiler introspection) when C is not enabled which is
# that the POSITION_INDEPENDENT_CODE ON target property for the Ada
# library disappears if cmake is rerun.

project(test_ada C Ada)

# Identify this project as the core build (as opposed to installed example
# build).

set(CORE_BUILD ON)

# Build and test build-tree example?
option(BUILD_TEST "Compile example in the build tree and test it" OFF)

option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# Install locations

set(
CMAKE_INSTALL_EXEC_PREFIX
${CMAKE_INSTALL_PREFIX}
CACHE PATH "install location for architecture-dependent files"
)

set(
CMAKE_INSTALL_BINDIR
${CMAKE_INSTALL_EXEC_PREFIX}/bin
CACHE PATH "install location for user executables"
)

set(
CMAKE_INSTALL_DATADIR
${CMAKE_INSTALL_PREFIX}/share
CACHE PATH "install location for read-only architecture-independent data"
)

set(
CMAKE_INSTALL_LIBDIR
${CMAKE_INSTALL_EXEC_PREFIX}/lib
CACHE PATH "install location for object code libraries"
)

# Configured test_ada install locations determined from user-updatable
# cached values above.

# Data.
set(DATA_DIR ${CMAKE_INSTALL_DATADIR}/test_ada)

# Libraries.
set(LIB_DIR ${CMAKE_INSTALL_LIBDIR})

# Binaries.
set(BIN_DIR ${CMAKE_INSTALL_BINDIR})

# Ada source files (*.adb, *.ads) (following recommendations in
# http://www.ada-france.org/debian/debian-ada-policy.html
set(ADA_INCLUDE_DIR ${CMAKE_INSTALL_DATADIR}/ada/adainclude/test_ada)

# Ada library information files (*.ali) (following recommendations in
# http://www.ada-france.org/debian/debian-ada-policy.html
set(ADA_LIB_DIR ${CMAKE_INSTALL_LIBDIR}/ada/adalib/test_ada)

set(LIB_INSTALL_RPATH ${LIB_DIR})

option(USE_RPATH "Use -rpath when linking libraries, executables" ON)

if(BUILD_SHARED_LIBS AND (WIN32 OR CYGWIN))
  # For platforms (currently WIN32 or Cygwin, although the Cygwin version
  # of CMake may support this in future since -rpath apparently does work
  # on that platform) where CMake does not use -rpath, use a workaround
  # where all dll's are collected in the dll subdirectory of the build tree.
  set(USE_DLL_SUBDIRECTORY ON)
else(BUILD_SHARED_LIBS AND (WIN32 OR CYGWIN))
  set(USE_DLL_SUBDIRECTORY OFF)
endif(BUILD_SHARED_LIBS AND (WIN32 OR CYGWIN))

# For all windows platforms all created dlls are gathered in the dll directory
# if you add this directory to your PATH all shared libraries are available
if(USE_DLL_SUBDIRECTORY)
  set(LIBRARY_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR}/dll)
endif(USE_DLL_SUBDIRECTORY)

set(LANG_SUPPORT_FILES
  ./:CMakeAdaCompiler.cmake.in
  ./:CMakeAdaInformation.cmake
  ./:CMakeDetermineAdaCompiler.cmake
  ./:CMakeTestAdaCompiler.cmake
  Platform:CYGWIN-GNU-Ada.cmake
  Platform:Darwin-GNU-Ada.cmake
  Platform:Linux-GNU-Ada.cmake
  Platform:Windows-GNU-Ada.cmake
  Compiler:GNU-Ada.cmake
  )

foreach(LANG_SUPPORT_info ${LANG_SUPPORT_FILES})
  string(REGEX REPLACE "^(.*):.*$" "\\1" LANG_SUPPORT_DIR ${LANG_SUPPORT_info})
  string(REGEX REPLACE "^.*:(.*)$" "\\1" LANG_SUPPORT_FILE ${LANG_SUPPORT_info})
  install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/Modules/${LANG_SUPPORT_DIR}/${LANG_SUPPORT_FILE}
    DESTINATION ${DATA_DIR}/examples/cmake/Modules/${LANG_SUPPORT_DIR}
    )
endforeach(LANG_SUPPORT_info ${LANG_SUPPORT_FILES})

# cmp executable is used to help implement test_noninteractive targed for
# Ada executable.
find_program(CMP_EXECUTABLE cmp)

# Configure and install a file to configure the build of the
# installed example.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/test_ada_configure.cmake_installed_examples.in
  ${CMAKE_CURRENT_BINARY_DIR}/test_ada_configure.cmake_installed_examples
  @ONLY
  )
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/test_ada_configure.cmake_installed_examples
  DESTINATION ${DATA_DIR}/examples/cmake/Modules
  RENAME test_ada_configure.cmake
  )

# Set up Ada variables specific to this project.
include(ada.cmake)

# Build Ada library
add_subdirectory(src_lib)
# Build Ada executable that links to that Ada library
add_subdirectory(src_executable)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/installed_CMakeLists.txt
  DESTINATION ${DATA_DIR}/examples
  RENAME CMakeLists.txt
  )
