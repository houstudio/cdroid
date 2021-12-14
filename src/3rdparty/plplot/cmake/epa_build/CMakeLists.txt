# Top-level CMakeLists.txt file for the epa_build project that simply builds
# and installs external projects configured in the subdirectories
# below this one using ExternalProject_Add commands configured with
# the appropriate build, test, and dependency information.

# Copyright (C) 2013-2017 Alan W. Irwin

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

project(epa_build NONE)

message(STATUS "CMake version = ${CMAKE_VERSION}")
message(STATUS "CMAKE_SYSTEM = ${CMAKE_SYSTEM}")
message(STATUS "CMAKE_GENERATOR = ${CMAKE_GENERATOR}")
message(STATUS "CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX}")

cmake_minimum_required(VERSION 3.0.2 FATAL_ERROR)

set(EPA_BASE ${CMAKE_BINARY_DIR}/epa_build)
message(STATUS "EPA_BASE = ${EPA_BASE}")
# We keep the variable name as EPA_BASE (to be used in the subdirectories
# along with all the other variables with EPA_ suffix.  But the property
# name must be EP_BASE in order for ExternalProject_Add to work correctly.

set_directory_properties(PROPERTIES EP_BASE ${EPA_BASE})

# Set epa_build variables to control builds in general

# Test whether there is access to the MSYS platform on the PATH.  Note
# this is a different question than whether you are using the "MSYS
# Makefiles" generator since some other generators (e.g., "MinGW
# Makefiles") can have MSYS on the path so long as sh.exe is renamed
# or removed from that PATH.  Note, we search for the automatic
# installer app, mingw-get.exe and the bash.exe shell to check
# (approximately) whether we have access to the MSYS platform, and
# then we test that platform (very approximately) for completeness by
# making sure unzip (necessary for some build configurations to unpack
# their packages) is installed (using mingw-get).

find_program(MINGW_GET_EXECUTABLE mingw-get)
find_program(BASH_EXECUTABLE bash)

if(MINGW_GET_EXECUTABLE AND BASH_EXECUTABLE AND NOT CYGWIN)
  set(MSYS_PLATFORM ON)
else(MINGW_GET_EXECUTABLE AND BASH_EXECUTABLE AND NOT CYGWIN)
  set(MSYS_PLATFORM OFF)
endif(MINGW_GET_EXECUTABLE AND BASH_EXECUTABLE AND NOT CYGWIN)

message(STATUS "WIN32 = ${WIN32}")
message(STATUS "MINGW = ${MINGW}")
message(STATUS "MSYS = ${MSYS}")
message(STATUS "MSYS_PLATFORM = ${MSYS_PLATFORM}")
message(STATUS "CYGWIN = ${CYGWIN}")

if(WIN32 AND NOT MSYS_PLATFORM)
  message(FATAL_ERROR "epa_build does not work correctly on Windows without the MSYS platform.  Install the MSYS platform (perhaps without sh.exe depending on CMake generator) on Windows.")
endif(WIN32 AND NOT MSYS_PLATFORM)

# List of executables required by epa_build CMake logic.
set(executables_LIST
  # ExternalProject workarounds
  touch
  # Unpacking
  unzip
  # Updating
  mkdir
  cp
  patch
  # General
  env
  # Not specifically required by epa_build (yet), but some
  # individual project configurations will not work unless this is
  # available so might as well check it here.
  bash
  make
  python
  uname
  sed
  ln
  chmod
  )

foreach(executable ${executables_LIST})
  string(TOUPPER ${executable} EXECUTABLE)
  find_program(${EXECUTABLE}_EXECUTABLE ${executable})
  if(NOT ${EXECUTABLE}_EXECUTABLE)
    if(MSYS_PLATFORM)
      message(STATUS "${executable} missing from your MSYS platform.  Use mingw-get to install it.")
    endif(MSYS_PLATFORM)
    message(FATAL_ERROR "${executable} must be on your PATH in order for epa_build to work correctly")
  endif(NOT ${EXECUTABLE}_EXECUTABLE)
endforeach(executable ${executables_LIST})

# Get the Python version.
execute_process(
  COMMAND
  ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_python_version())"
  OUTPUT_VARIABLE PYTHON_VERSION
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
message(STATUS "PYTHON_VERSION = ${PYTHON_VERSION}")

# Determine whether OS is 64-bits from uname -m "machine name" field.

execute_process(
  COMMAND
  ${UNAME_EXECUTABLE} -m
  OUTPUT_VARIABLE UNAME_MACHINE
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )

# Determine desired install permissions on shared objects.
if(CMAKE_INSTALL_SO_NO_EXE)
  set(SO_NUMERICAL_PERMISSIONS 644)
else(CMAKE_INSTALL_SO_NO_EXE)
  set(SO_NUMERICAL_PERMISSIONS 755)
endif(CMAKE_INSTALL_SO_NO_EXE)

# This will need refinement as more platforms tested, but it works on
# Intel/AMD Linux hardware and also MinGW/MSYS on 32-bit Windows for
# now.
if(UNAME_MACHINE STREQUAL x86_64)
  set(EPA_HAVE_64_BIT_OS ON)
else(UNAME_MACHINE STREQUAL x86_64)
  set(EPA_HAVE_64_BIT_OS OFF)
endif(UNAME_MACHINE STREQUAL x86_64)

# Use modified version of the CMake 2.8.12 ExternalProject module
# where the tar.xz processing has been fixed.
set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR})
include(ExternalProject)

if(MSYS_PLATFORM)
  # Useful function to convert Windows list of semicolon-delimited
  # PATHs to the equivalent list of MSYS PATHs (exactly like the
  # colon-delimited Unix list of PATHs except the driver letters are
  # specified as the initial one-character component of each of the
  # PATHs).  For example, this function will transform the Windows
  # list of PATHs, "z:\path1;c:\path2" to "/z/path1:/c/path2".
  function(determine_msys_path MSYS_PATH NATIVE_PATH)
    #message(STATUS "NATIVE_PATH = ${NATIVE_PATH}")
    string(REGEX REPLACE "^\([a-zA-z]\):" "/\\1" PATH  "${NATIVE_PATH}")
    string(REGEX REPLACE ";\([a-zA-z]\):" ";/\\1" PATH  "${PATH}")
    string(REGEX REPLACE ";" ":" PATH  "${PATH}")
    file(TO_CMAKE_PATH "${PATH}" PATH)
    #message(STATUS "MSYS_PATH = ${PATH}")
    set(${MSYS_PATH} ${PATH} PARENT_SCOPE)
  endfunction(determine_msys_path)
endif(MSYS_PLATFORM)

function(epa_boilerplate
    _ignored_dependencies_LIST
    _PACKAGE
    _dependencies_LIST
    _dependencies_targets
    _EPA_PATH
    _source_PATH
    )

  #message(STATUS "DEBUG DEPENDENCY HANDLING: entering ${_PACKAGE} = ${${_PACKAGE}}")
  get_property(saved_started_subdirectories_LIST GLOBAL PROPERTY EPA_started_subdirectories_LIST)
  set_property(GLOBAL APPEND PROPERTY EPA_started_subdirectories_LIST ${${_PACKAGE}})
  get_property(started_subdirectories_LIST GLOBAL PROPERTY EPA_started_subdirectories_LIST)
  # Remove dependencies that should be ignored.
  if(${_dependencies_LIST} AND ${_ignored_dependencies_LIST})
    list(REMOVE_ITEM ${_dependencies_LIST} ${${_ignored_dependencies_LIST}})
  endif(${_dependencies_LIST} AND ${_ignored_dependencies_LIST})

# Check for missing configurations and eliminate those from ${_dependencies_LIST}.
  foreach(build_configuration ${${_dependencies_LIST}})
    if(NOT EXISTS ${CMAKE_SOURCE_DIR}/${build_configuration}/CMakeLists.txt)
      message(STATUS "Warning: A build_configuration for ${build_configuration} does not exist so it is assumed this dependency of ${${_PACKAGE}} has been installed on your platform by means other than epa_build.")
      list(REMOVE_ITEM ${_dependencies_LIST} ${build_configuration})
    endif(NOT EXISTS ${CMAKE_SOURCE_DIR}/${build_configuration}/CMakeLists.txt)
  endforeach(build_configuration ${${_dependences_LIST}})

  #message(STATUS "DEBUG DEPENDENCY HANDLING: ${_PACKAGE} = ${${_PACKAGE}} has the following list of expurgated dependencies: ${${_dependencies_LIST}}")

  # Initial value of finished_subdirectories_LIST required for following
  # loop logic to work correctly.
  get_property(finished_subdirectories_LIST GLOBAL PROPERTY EPA_finished_subdirectories_LIST)
  foreach(build_configuration ${${_dependencies_LIST}})
    list(FIND started_subdirectories_LIST ${build_configuration} index)
    if(index GREATER -1)
      message(FATAL_ERROR "Circular dependency: package ${build_configuration} depends on package ${${_PACKAGE}} which depends on package ${build_configuration}.")
    endif(index GREATER -1)

    list(FIND finished_subdirectories_LIST ${build_configuration} index)
    if(index LESS 0)
      # Only if build_configuration is not in finished_subdirectories_LIST.
      add_subdirectory(
	${CMAKE_SOURCE_DIR}/${build_configuration}
	${CMAKE_BINARY_DIR}/${build_configuration}
	)
      # GLOBAL property EPA_finished_subdirectories_LIST has the subdirectory
      # name appended to it for the above add_subdirectory call.
      # So must update finished_subdirectories_LIST
      # in order for the next test of build_configuration to be valid.
      get_property(finished_subdirectories_LIST GLOBAL PROPERTY EPA_finished_subdirectories_LIST)
    endif(index LESS 0)
  endforeach(build_configuration ${${_dependencies_LIST}})

  set(${_dependencies_targets})
  foreach(build_configuration ${${_dependencies_LIST}})
    list(APPEND ${_dependencies_targets} build_${build_configuration})
  endforeach(build_configuration ${${_dependences_LIST}})

  # Data that is related to the PATH that must be used.
  if(MSYS_PLATFORM)
    determine_msys_path(${_EPA_PATH} "${${_EPA_PATH}}")
    # Must have all elements of env command in MSYS platform form
    determine_msys_path(${_source_PATH} "${EPA_BASE}/Source/build_${${_PACKAGE}}")
  else(MSYS_PLATFORM)
    set(${_source_PATH} "${EPA_BASE}/Source/build_${${_PACKAGE}}")
  endif(MSYS_PLATFORM)
  #message(STATUS "DEBUG: (modified for ${${_PACKAGE}}) ${_EPA_PATH} = ${${_EPA_PATH}}")

  # Update global properties consistent with a successful conclusion of this
  # function call.
  set_property(GLOBAL PROPERTY EPA_started_subdirectories_LIST ${saved_started_subdirectories_LIST})

  set_property(GLOBAL APPEND PROPERTY EPA_build_targets_LIST build_${${_PACKAGE}})
  set_property(GLOBAL APPEND PROPERTY EPA_finished_subdirectories_LIST ${${_PACKAGE}})
  # Temporary debugging of dependencies.
  #get_property(finished_subdirectories_LIST GLOBAL PROPERTY EPA_finished_subdirectories_LIST)
  #message(STATUS "DEBUG DEPENDENCY HANDLING: finished_subdirectories_LIST = ${finished_subdirectories_LIST}")

  # Propagate changed output arguments to parent scope of function.
  set(${_dependencies_LIST} ${${_dependencies_LIST}} PARENT_SCOPE)
  set(${_dependencies_targets} ${${_dependencies_targets}} PARENT_SCOPE)
  set(${_EPA_PATH} ${${_EPA_PATH}} PARENT_SCOPE)
  set(${_source_PATH} ${${_source_PATH}} PARENT_SCOPE)
  #message(STATUS "DEBUG DEPENDENCY HANDLING: leaving ${_PACKAGE} = ${${_PACKAGE}}")
endfunction(epa_boilerplate)

# This gives the full pathname of the associated build tool for at
# least the "Unix Makefiles", "Ninja", "MSYS Makefiles", "MinGW
# Makefiles", and "NMake Makefiles JOM" CMake generators.
set(EPA_BUILD_COMMAND "${CMAKE_MAKE_PROGRAM}")

# Actual make programme used for autotools builds.
set(EPA_MAKE_COMMAND ${MAKE_EXECUTABLE})

set(EPA_CTEST_COMMAND "${CMAKE_CTEST_COMMAND}")

if(MSYS_PLATFORM)
  # On the MSYS platform, the env command (used to set relevant
  # environment variables for the commands below in the */CMakeLists.txt
  # scripts) requires all full paths be in the MSYS platform form.
  determine_msys_path(EPA_BUILD_COMMAND "${EPA_BUILD_COMMAND}")
  determine_msys_path(EPA_MAKE_COMMAND "${EPA_MAKE_COMMAND}")
  determine_msys_path(EPA_CTEST_COMMAND "${EPA_CTEST_COMMAND}")
  determine_msys_path(EPA_CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")
  determine_msys_path(MSYS_CMAKE_COMMAND "${CMAKE_COMMAND}")
  # Propagate the overall CMake generator and install prefix to all CMake-based
  # software project builds.
  set(EPA_CMAKE_COMMAND ${MSYS_CMAKE_COMMAND} "-G${CMAKE_GENERATOR}" -DCMAKE_INSTALL_PREFIX:PATH=${EPA_CMAKE_INSTALL_PREFIX})
  # Propagate CMAKE_C_COMPILER, CMAKE_CXX_COMPILER, and
  # CMAKE_RC_COMPILER to all CMake-based software project builds.
  # (jom currently requires this as a workaround).
  if(CMAKE_C_COMPILER)
    determine_msys_path(MSYS_CMAKE_C_COMPILER "${CMAKE_C_COMPILER}")
    list(APPEND EPA_CMAKE_COMMAND -DCMAKE_C_COMPILER:FILEPATH=${MSYS_CMAKE_C_COMPILER})
  endif(CMAKE_C_COMPILER)
  if(CMAKE_CXX_COMPILER)
    determine_msys_path(MSYS_CMAKE_CXX_COMPILER "${CMAKE_CXX_COMPILER}")
    list(APPEND EPA_CMAKE_COMMAND -DCMAKE_CXX_COMPILER:FILEPATH=${MSYS_CMAKE_CXX_COMPILER})
  endif(CMAKE_CXX_COMPILER)
  if(CMAKE_RC_COMPILER)
    determine_msys_path(MSYS_CMAKE_RC_COMPILER "${CMAKE_RC_COMPILER}")
    list(APPEND EPA_CMAKE_COMMAND -DCMAKE_RC_COMPILER:FILEPATH=${MSYS_CMAKE_RC_COMPILER})
  endif(CMAKE_RC_COMPILER)
  # No extra dropped dependencies for the MSYS_PLATFORM case.
  set(extra_ignored_dependencies_list)
else(MSYS_PLATFORM)
  # Propagate the overall CMake generator and install prefix to all CMake-based
  # software project builds.
  set(EPA_CMAKE_COMMAND ${CMAKE_COMMAND} "-G${CMAKE_GENERATOR}" -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_INSTALL_PREFIX})
  set(EPA_CMAKE_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

  # Drop xmlcatalog-wrapper dependency since that is only suitable for
  # the MSYS_PLATFORM case.
  set(extra_ignored_dependencies_list xmlcatalog-wrapper)
endif(MSYS_PLATFORM)

# Propagate the install prefix to autotools-based builds
set(EPA_CONFIGURE_COMMAND configure --prefix=${EPA_CMAKE_INSTALL_PREFIX})

if(CMAKE_VERBOSE_MAKEFILE)
  # Not sure whether all contemplated back-ends support this or not, but
  # most should.
  list(APPEND EPA_CMAKE_COMMAND -DCMAKE_VERBOSE_MAKEFILE=ON)
  # Some autotools build projects give verbose make results when V=1
  # and I don't think it will interfere with the rest.
  list(APPEND EPA_MAKE_COMMAND V=1)
endif(CMAKE_VERBOSE_MAKEFILE)

# These options run the PLplot comprehensive test script (for
# either/both the noninteractive and interactive subsets of the tests)
# for the exact same build environment (i.e., cmake options,
# environment variables, buildtools that have been built, and
# dependencies that have been built) as used for the epa_build of
# plplot and plplot_lite.
option(COMPREHENSIVE_PLPLOT_TEST_NONINTERACTIVE "Use comprehensive noninteractive test for PLplot (which requires no interaction by the user but does require substantial CPU time and ~4GB of disk space)" OFF)

option(COMPREHENSIVE_PLPLOT_TEST_INTERACTIVE "Use comprehensive interactive test for PLplot (which requires interaction by the user to click through some of the tests)" OFF)

# The parallel versions are for software packages that
# do not have race conditions for parallel builds or tests.

set(NUMBER_PARALLEL_JOBS 4 CACHE STRING "Number of parallel jobs")

set(EPA_PARALLEL_BUILD_COMMAND "${EPA_BUILD_COMMAND}" -j${NUMBER_PARALLEL_JOBS})
set(EPA_PARALLEL_CTEST_COMMAND "${EPA_CTEST_COMMAND}" -j${NUMBER_PARALLEL_JOBS})
# For autotools based builds.
option(AUTOTOOLS_PARALLEL_BUILD "Build autotools projects using parallel make unless a specific project is expressely configured otherwise" ON)
if(AUTOTOOLS_PARALLEL_BUILD)
  set(EPA_PARALLEL_MAKE_COMMAND "${EPA_MAKE_COMMAND}" -j${NUMBER_PARALLEL_JOBS})
else(AUTOTOOLS_PARALLEL_BUILD)
  set(EPA_PARALLEL_MAKE_COMMAND "${EPA_MAKE_COMMAND}")
endif(AUTOTOOLS_PARALLEL_BUILD)

message(STATUS "EPA_CMAKE_COMMAND = ${EPA_CMAKE_COMMAND}")
message(STATUS "EPA_CONFIGURE_COMMAND = ${EPA_CONFIGURE_COMMAND}")
message(STATUS "EPA_BUILD_COMMAND = ${EPA_BUILD_COMMAND}")
message(STATUS "EPA_PARALLEL_BUILD_COMMAND = ${EPA_PARALLEL_BUILD_COMMAND}")
message(STATUS "EPA_MAKE_COMMAND = ${EPA_MAKE_COMMAND}")
message(STATUS "EPA_PARALLEL_MAKE_COMMAND = ${EPA_PARALLEL_MAKE_COMMAND}")
message(STATUS "EPA_CTEST_COMMAND = ${EPA_CTEST_COMMAND}")
message(STATUS "EPA_PARALLEL_CTEST_COMMAND = ${EPA_PARALLEL_CTEST_COMMAND}")

# Put ${CMAKE_INSTALL_PREFIX}/bin on the PATH as well for those cases
# where some executable built and installed by epa_build needs
# to be found by another project being configured by epa_build.
if(MSYS_PLATFORM)
  set(BP_ORIGINAL_NATIVE_PATH "$ENV{PATH};${CMAKE_INSTALL_PREFIX}/bin")
else(MSYS_PLATFORM)
  set(BP_ORIGINAL_NATIVE_PATH "$ENV{PATH}:${CMAKE_INSTALL_PREFIX}/bin")
endif(MSYS_PLATFORM)
set(EPA_PATH "${BP_ORIGINAL_NATIVE_PATH}")

# This option not used currently, but instead of removing it, comment
# it out because it might be useful later if we ever configure a project
# where we want a choice between an ordinary download and repository access.
# option(PREFER_DOWNLOAD "Prefer to obtain source code for projects using a simple download of tarball rather than via svn, git, hg, or bzr repository access" ON)

# Now include the build configurations for certain targets and their
# dependencies.

option(BUILD_THE_BUILDTOOLS "Build the buildtools (such as cmake) used for the rest of the builds" OFF)

if(BUILD_THE_BUILDTOOLS)
  set(executables_LIST
    sh
    )
  foreach(executable ${executables_LIST})
    string(TOUPPER ${executable} EXECUTABLE)
    find_program(${EXECUTABLE}_EXECUTABLE ${executable})
    if(NOT ${EXECUTABLE}_EXECUTABLE)
      if(MSYS_PLATFORM)
	message(STATUS "${executable} missing from your MSYS platform.  If sh is missing, use a generator other than 'MinGW Makefiles'.  If something else is missing use mingw-get to install it.")
      endif(MSYS_PLATFORM)
      message(FATAL_ERROR "${executable} must be on your PATH in order for epa_build to work correctly")
    endif(NOT ${EXECUTABLE}_EXECUTABLE)
  endforeach(executable ${executables_LIST})

  # List sufficient buildtool configurations so they will suck in
  # the remaining buildtool configurations via dependencies.
  # Order does not matter because the dependency logic takes
  # care of any ordering issues.
  set(subdirectories_LIST
    pkg-config
    ##subversion
    swig
    tk
    # itcl version 4 is integrated into tcl
    # itk version 4 is not integrated into tk (or tcl).
    itk
    # iwidgets (version 4.1.0)
    iwidgets
    # itcl version 3 is an independent project
    itcl3
    # itk version 3 is an independent project
    itk3
    # iwidgets (version 4.0.1)
    iwidgets4.0
    )
  if(NOT MSYS_PLATFORM)
    # Required system version of libcurl, an essential cmake build
    # dependency, is not currently (until the epa_build project
    # configures that build) available for the MSYS_PLATFORM.  So only
    # allow the potential to build the cmake buildtool for the
    # non-MSYS platform case.  And for the MSYS platform case use a
    # binary version of CMake supplied by Kitware.
    list(APPEND subdirectories_LIST cmake)
    # epa_build of lapack_blas and valgrind not tested on MSYS_PLATFORM yet
    # so add them here for the time being.
    list(APPEND subdirectories_LIST lapack_blas valgrind)
  endif(NOT MSYS_PLATFORM)
else(BUILD_THE_BUILDTOOLS)
  # List sufficient normal package configurations so they will suck in
  # the remaining configurations via dependencies.
  # Order does not matter because the dependency logic takes
  # care of any ordering issues.
  set(subdirectories_LIST
    ndiff
    uncrustify
    plplot
    plplot_lite
    gtk+
    libLASi
    wxwidgets
    # qt4_lite install interferes with qt5_lite build and vice versa.
    # Therefore, we have commented out the build configuration of
    # qt4_lite and have chosen to use the build_configuration of
    # qt5_lite instead.
    # qt4_lite
    qt5_lite
    freetype
    )
  if(NOT MSYS_PLATFORM)
    # octave_lite install depends on lapack_blas install with
    # build tools but that only available for the current case:
    list(APPEND subdirectories_LIST octave_lite)
  endif(NOT MSYS_PLATFORM)
endif(BUILD_THE_BUILDTOOLS)

set_property(GLOBAL PROPERTY EPA_started_subdirectories_LIST)
set_property(GLOBAL PROPERTY EPA_finished_subdirectories_LIST)
set_property(GLOBAL PROPERTY EPA_build_targets_LIST)

#message(STATUS "DEBUG DEPENDENCY HANDLING (for outer subdirectories loop): subdirectories_LIST = ${subdirectories_LIST}")

# Using a while loop seems obvious here
# because each add_subdirectory command can generate others
# so that several elements of the subdirectories list could be knocked
# off per loop iteration.  However, that is not a good idea in
# practice because it turns out that fatal CMake errors don't stop the
# loop (which I presume is a bug) so it just continues indefinitely.  Ugh!
# So use foreach RANGE loop instead with maximum number of loop
# interations equal to LENGTH_subdirectories_LIST and break out of it
# early if necessary if the subdirectories_LIST has become exhausted.
list(LENGTH subdirectories_LIST LENGTH_subdirectories_LIST)
foreach(idummy RANGE ${LENGTH_subdirectories_LIST})
  list(GET subdirectories_LIST 0 subdirectory)
  # This uses EPA_started_subdirectories_LIST internally to check for
  # circular dependencies, and updates EPA_finished_subdirectories_LIST
  # and EPA_build_targets_LIST that are used here.
  add_subdirectory(${subdirectory})
  get_property(finished_subdirectories_LIST GLOBAL PROPERTY EPA_finished_subdirectories_LIST)
  list(REMOVE_ITEM subdirectories_LIST ${finished_subdirectories_LIST})
#  message(STATUS "DEBUG DEPENDENCY HANDLING (for outer subdirectories loop): visited ${subdirectory}")
#  message(STATUS "DEBUG DEPENDENCY HANDLING (for outer subdirectories loop): subdirectories_LIST = ${subdirectories_LIST}")
  list(LENGTH subdirectories_LIST LENGTH_subdirectories_LIST)
  if(LENGTH_subdirectories_LIST EQUAL 0)
    break()
  endif(LENGTH_subdirectories_LIST EQUAL 0)
endforeach(idummy RANGE ${LENGTH_subdirectories_LIST})

add_custom_target(build_all)
get_property(build_targets_LIST GLOBAL PROPERTY EPA_build_targets_LIST)
message(STATUS "The target build_all has the following dependent targets: ${build_targets_LIST}")
add_dependencies(build_all ${build_targets_LIST})
