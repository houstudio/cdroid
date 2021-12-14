# Top-level CMakeLists.txt for a simple project consisting of a
# "Hello, world" library and executable to test CMake language support
# for Fortran. See the README file in this directory for more details.

# MAINTENANCE
# Use same minimum version for all platforms as the non-Linux platform minimum adopted for
# the PLplot project.
cmake_minimum_required(VERSION 3.11.0 FATAL_ERROR)

project(test_fortran Fortran)

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

# Configured test_fortran install locations determined from user-updatable
# cached values above.

# Data.
set(DATA_DIR ${CMAKE_INSTALL_DATADIR}/test_fortran)

# Libraries.
set(LIB_DIR ${CMAKE_INSTALL_LIBDIR})

# Binaries.
set(BIN_DIR ${CMAKE_INSTALL_BINDIR})

set(LIB_INSTALL_RPATH ${LIB_DIR})

# Module files.
set(FORTRAN_MOD_DIR ${LIB_DIR}/fortran/modules/test_fortran)

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

# cmp executable is used to help implement test_noninteractive targed for
# the Fortran executable.
find_program(CMP_EXECUTABLE cmp)

# Configure and install a file to configure the build of the
# installed example.
configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/test_fortran_configure.cmake_installed_examples.in
  ${CMAKE_CURRENT_BINARY_DIR}/test_fortran_configure.cmake_installed_examples
  @ONLY
  )
install(FILES
  ${CMAKE_CURRENT_BINARY_DIR}/test_fortran_configure.cmake_installed_examples
  DESTINATION ${DATA_DIR}/examples
  RENAME test_fortran_configure.cmake
  )

# Build Fortran "Hello, world" library
add_subdirectory(src_lib)
# Build Fortran executable that links to that Fortran library
add_subdirectory(src_executable)

install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/installed_CMakeLists.txt
  DESTINATION ${DATA_DIR}/examples
  RENAME CMakeLists.txt
  )
