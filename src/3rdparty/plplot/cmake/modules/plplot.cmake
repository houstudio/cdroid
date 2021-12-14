# cmake/modules/plplot.cmake
#
# Copyright (C) 2006-2018 Alan W. Irwin
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

# Module for determining all configuration variables for PLplot.

# The "certain devices" I have in mind at the moment are wxwidgets and wingcc since both
# of those devices simply show a blank screen rather than the plot actively being rendered when -np is
# used.  So once those -np issues are fixed for wxwidgets and wingcc, this option will no longer
# be necessary.

option(PAUSE_CERTAIN_INTERACTIVE_DEVICES "Experimental option to drop the -np option for interactive tests of certain devices where -np is currently not implemented well" OFF)

option(ADD_SPECIAL_CONSISTENCY_CHECKING "Add extra source-tree consistency checking targets that require special tools" OFF)

# option(DROP_GTK_PLUS_2_BUILDS "Drop builds of gtk+2-only targets that clash with gtk+3 dependencies" OFF)
set(DROP_GTK_PLUS_2_BUILDS ON CACHE BOOL "Drop builds of gtk+2-only targets that clash with gtk+3 dependencies" FORCE)

# target that depends on all check targets.
add_custom_target(check_all)

# libraries are all shared by default
option(BUILD_SHARED_LIBS "Build shared libraries" ON)

# CYGWIN eliminated here because .a suffix is not sufficient to guarantee a
# static library on that platform.
if(UNIX AND NOT CYGWIN AND NOT BUILD_SHARED_LIBS)
  option(FORCE_EXTERNAL_STATIC "Force external UNIX libraries to be static" OFF)
else(UNIX AND NOT CYGWIN AND NOT BUILD_SHARED_LIBS)
  set(FORCE_EXTERNAL_STATIC OFF CACHE BOOL "Force external UNIX libraries to be static" FORCE)
endif(UNIX AND NOT CYGWIN AND NOT BUILD_SHARED_LIBS)

if(FORCE_EXTERNAL_STATIC)
  # All find_library commands append this suffix to the searched name so
  # this always forces find_library to find only the Unix static library
  # if it exists or fail if it doesn't exist.
  set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
endif(FORCE_EXTERNAL_STATIC)

if(BUILD_SHARED_LIBS)
  # OFF means shared libraries (but never static libraries) are
  # transitively linked by default by CMake and also by pkg-config (used
  # to generate compile and link flags for the traditional build and
  # test system of the installed examples).
  if(WIN32_OR_CYGWIN)
    option(NON_TRANSITIVE "Option to use non-transitive linking" OFF)
  else(WIN32_OR_CYGWIN)
    option(NON_TRANSITIVE "Option to use non-transitive linking" ON)
  endif(WIN32_OR_CYGWIN)
else(BUILD_SHARED_LIBS)
  set(NON_TRANSITIVE OFF CACHE BOOL "Option to use non-transitive linking" FORCE)
endif(BUILD_SHARED_LIBS)

# Color maps (discrete and continuous) to use by default
if(NOT DEFAULT_CMAP0_FILE)
  set(DEFAULT_CMAP0_FILE "cmap0_default.pal")
endif(NOT DEFAULT_CMAP0_FILE)

if(NOT DEFAULT_CMAP1_FILE)
  set(DEFAULT_CMAP1_FILE "cmap1_default.pal")
endif(NOT DEFAULT_CMAP1_FILE)

# Deprecated functions are not included by default. Set to ON if you
# require this for backwards compatibility.
option(PL_DEPRECATED "Include support for deprecated functions" OFF)

# Set to ON if want to use general fill_intersection_polygon approach
# rather than the traditional code to fill the intersection of a
# polygon with the clipping limits.

option(USE_FILL_INTERSECTION_POLYGON "use fill_intersection_polygon" OFF)

# Need these modules to do subsequent checks.
include(CheckIncludeFile)
include(CheckIncludeFiles)
include(CheckFunctionExists)
include(CheckSymbolExists)
include(CheckPrototypeExists)
# Required for the cmake_push_check_state and cmake_pop_check_state
# macros used to store and retrieve CMAKE_REQUIRED_* variables used in
# conjunction with some of the CMake check_* functions that we call.
include(CMakePushCheckState)

# Include some useful functions that Plplot requires.
include(plplot_functions)

# =======================================================================
# Compilation and build options (PLFLT, install locations, and rpath)
# Note, must come before java since that depends on, e.g., LIB_DIR.
# =======================================================================

# WIN32_OR_CYGWIN covers CYGWIN as well (and possibly MINGW, but we will make sure).
if(WIN32_OR_CYGWIN OR MINGW)
  set(EXEEXT .exe)
endif(WIN32_OR_CYGWIN OR MINGW)

include(double)
include(instdirs)
include(rpath)

# Common CMakeLists.txt files are used to build the examples in the build
# tree and also to build the installed examples by the new CMake-based build
# system devoted to that purpose. Set this fundamental identification to
# distinguish the two cases.
set(CORE_BUILD ON)

option(BUILD_TEST "Compile examples in the build tree and enable ctest" OFF)

# Use bash when available for ctest and install tree test support
find_program(SH_EXECUTABLE bash)
find_program(SH_EXECUTABLE win-bash)
if(SH_EXECUTABLE)
  set(HAVE_BASH ON)
else(SH_EXECUTABLE)
  find_program(SH_EXECUTABLE sh)
endif(SH_EXECUTABLE)
message(STATUS "SH_EXECUTABLE = ${SH_EXECUTABLE}")

if(NOT SH_EXECUTABLE)
   message(STATUS
   "WARNING: bash shell not found, ctest will not work properly"
   )
endif(NOT SH_EXECUTABLE)

option(PREBUILD_DIST "Pre-build all components required for distribution" OFF)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(
  PREBUILD_DIST OFF CACHE INTERNAL
  "Pre-build all components required for distribution"
  )
endif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")

# create a devpackage file
option(DEVPAK "Create DevPackage" NO)
if(DEVPAK AND NOT WIN32_OR_CYGWIN)
  message( STATUS "DevPackage only available for Win32. Set DEVPAK to OFF." )
  set(DEVPAK OFF)
endif(DEVPAK AND NOT WIN32_OR_CYGWIN)
if(DEVPAK AND BUILD_TEST)
  message( STATUS "Examples are not build for DevPackage. Set BUILD_TEST to OFF." )
  set(BUILD_TEST OFF)
endif(DEVPAK AND BUILD_TEST)


# =======================================================================
# Headers
# =======================================================================

# AC_HEADER_STDC is gross overkill since the current PLplot code only uses
# this for whether or not atexit can be used.  But implement the full suite
# of AC_HEADER_STDC checks to keep the cmake version in synch with autotools
# and just in case some PLplot developer assumes the complete check for
# standard headers is done for a future programming change.
#
# From info autoconf....
# Define STDC_HEADERS if the system has ANSI C header files.
# Specifically, this macro checks for stdlib.h', stdarg.h',
# string.h', and float.h'; if the system has those, it probably
# has the rest of the ANSI C header files.  This macro also checks
# whether string.h' declares memchr' (and thus presumably the
# other mem' functions), whether stdlib.h' declare free' (and
# thus presumably malloc' and other related functions), and whether
# the ctype.h' macros work on characters with the high bit set, as
# ANSI C requires.

message(STATUS "Checking whether system has ANSI C header files")
check_include_files("stdlib.h;stdarg.h;string.h;float.h" StandardHeadersExist)
if(StandardHeadersExist)
  check_prototype_exists(memchr string.h memchrExists)
  if(memchrExists)
    check_prototype_exists(free stdlib.h freeExists)
    if(freeExists)
      include(TestForHighBitCharacters)
      if(CMAKE_HIGH_BIT_CHARACTERS)
        message(STATUS "ANSI C header files - found")
        set(STDC_HEADERS 1 CACHE INTERNAL "System has ANSI C header files")
      endif(CMAKE_HIGH_BIT_CHARACTERS)
    endif(freeExists)
  endif(memchrExists)
endif(StandardHeadersExist)
if(NOT STDC_HEADERS)
  message(STATUS "ANSI C header files - not found")
  set(STDC_HEADERS 0 CACHE INTERNAL "System has ANSI C header files")
endif(NOT STDC_HEADERS)

# AC_CHECK_HEADERS(unistd.h termios.h stdint.h)
check_include_files(unistd.h PL_HAVE_UNISTD_H)
check_include_files(termios.h HAVE_TERMIOS_H)
check_include_files(stdint.h PL_HAVE_STDINT_H)
check_include_file(crt_externs.h HAVE_CRT_EXTERNS_H)

# AC_HEADER_SYS_WAIT
include(TestForStandardHeaderwait)

# Reasonable approximation to AC_HEADER_DIRENT without the SCO stuff.
include(CheckDIRSymbolExists)
check_dirsymbol_exists("sys/types.h;dirent.h" HAVE_DIRENT_H)
if(NOT HAVE_DIRENT_H)
  check_dirsymbol_exists("sys/types.h;sys/ndir.h" HAVE_SYS_NDIR_H)
  if(NOT HAVE_SYS_NDIR_H)
    check_dirsymbol_exists("sys/types.h;sys/dir.h" HAVE_SYS_DIR_H)
    if(NOT HAVE_SYS_DIR_H)
      check_dirsymbol_exists("sys/types.h;ndir.h" HAVE_NDIR_H)
      if(NOT HAVE_NDIR_H AND UNIX)
        message(FATAL_ERROR
        "FATAL_ERROR for plplot.cmake: "
        "DIR symbol must be defined by Unix system headers."
        )
      endif(NOT HAVE_NDIR_H AND UNIX)
    endif(NOT HAVE_SYS_DIR_H)
  endif(NOT HAVE_SYS_NDIR_H)
endif(NOT HAVE_DIRENT_H)
# Note the above tests #include <sys/types.h> to follow how
# AC_HEADER_DIRENT does its testing.  Therefore, always do our
# own #defines that way for the cmake build system.  Note, this
# sys/types.h requirement occurs for Mac OS X and possibly other systems.
# It is possible it will go away in the future, but we will follow whatever
# is done by AC_HEADER_DIRENT here until that changes.
set(NEED_SYS_TYPE_H ON)

#=======================================================================
# Typedefs
#=======================================================================

# In the past, some X11 headers required "caddr_t" even on systems that
# claimed POSIX.1 compliance, which was illegal.  This made it impossible
# to compile programs that included X11 headers if _POSIX_SOURCE was
# defined.  We used to work around this potential problem by just defining
# caddr_t to 'char *' on all systems (unless it is set already), whether
# it was needed or not. Now we ignore the issue because we don't expect
# such broken X behaviour any more and because this kind of argument list
# for AC_CHECK_TYPE is now deprecated in the autoconf documentation.

# Do not implement the equivalent of this since commented out in the ABS
# system.
# AC_CHECK_TYPE(caddr_t, char *)

# Test signal handler return type (mimics AC_TYPE_SIGNAL)
include(TestSignalType)

include(CheckFunctionExists)
check_function_exists(popen HAVE_POPEN)
check_function_exists(usleep PL_HAVE_USLEEP)
check_function_exists(nanosleep PL_HAVE_NANOSLEEP)
check_function_exists(mkstemp PL_HAVE_MKSTEMP)
check_function_exists(mkdtemp PL_HAVE_MKDTEMP)
check_function_exists(mkfifo PL_HAVE_MKFIFO)
check_function_exists(unlink PL_HAVE_UNLINK)
check_function_exists(_NSGetArgc HAVE_NSGETARGC)

# Check for FP functions, including underscored version which
# are sometimes all that is available on windows

check_symbol_exists(isfinite "math.h" HAVE_ISFINITE_SYMBOL)
check_symbol_exists(finite "math.h" HAVE_FINITE_SYMBOL)
if(HAVE_ISFINITE_SYMBOL)
  set(PL_HAVE_ISFINITE ON)
else(HAVE_ISFINITE_SYMBOL)
  if(HAVE_FINITE_SYMBOL)
    set(PL_HAVE_FINITE ON)
  else(HAVE_FINITE_SYMBOL)
    check_function_exists(finite HAVE_FINITE_FUNCTION)
    if(HAVE_FINITE_FUNCTION)
      set(PL_HAVE_FINITE ON)
    else(HAVE_FINITE_FUNCTION)
      check_symbol_exists(_finite "math.h" HAVE__FINITE_SYMBOL)
      if(HAVE__FINITE_SYMBOL)
        set(PL__HAVE_FINITE ON)
      else(HAVE__FINITE_SYMBOL)
        check_function_exists(_finite HAVE__FINITE_FUNCTION)
        if(HAVE__FINITE_FUNCTION)
          set(PL__HAVE_FINITE ON)
        endif(HAVE__FINITE_FUNCTION)
      endif(HAVE__FINITE_SYMBOL)
    endif(HAVE_FINITE_FUNCTION)
  endif(HAVE_FINITE_SYMBOL)
endif(HAVE_ISFINITE_SYMBOL)
if(PL_HAVE_FINITE)
  set(PL_HAVE_ISFINITE ON)
endif(PL_HAVE_FINITE)
if(PL__HAVE_FINITE)
  set(PL_HAVE_ISFINITE ON)
endif(PL__HAVE_FINITE)

check_symbol_exists(isnan "math.h" HAVE_ISNAN_SYMBOL)
if(HAVE_ISNAN_SYMBOL)
  set(PL_HAVE_ISNAN ON)
else(HAVE_ISNAN_SYMBOL)
  check_function_exists(isnan HAVE_ISNAN_FUNCTION)
  if(HAVE_ISNAN_FUNCTION)
    set(PL_HAVE_ISNAN ON)
  else(HAVE_ISNAN_FUNCTION)
    check_symbol_exists(_isnan "math.h" HAVE__ISNAN_SYMBOL)
    if(HAVE__ISNAN_SYMBOL)
      set(PL__HAVE_ISNAN ON)
    else(HAVE__ISNAN_SYMBOL)
      check_function_exists(_isnan HAVE__ISNAN_FUNCTION)
      if(HAVE__ISNAN_FUNCTION)
        set(PL__HAVE_ISNAN ON)
      endif(HAVE__ISNAN_FUNCTION)
    endif(HAVE__ISNAN_SYMBOL)
  endif(HAVE_ISNAN_FUNCTION)
endif(HAVE_ISNAN_SYMBOL)
if(PL__HAVE_ISNAN)
  set(PL_HAVE_ISNAN ON)
endif(PL__HAVE_ISNAN)

check_symbol_exists(isinf "math.h" HAVE_ISINF_SYMBOL)
if(HAVE_ISINF_SYMBOL)
  set(PL_HAVE_ISINF ON)
else(HAVE_ISINF_SYMBOL)
  check_function_exists(isinf HAVE_ISINF_FUNCTION)
  if(HAVE_ISINF_FUNCTION)
    set(PL_HAVE_ISINF ON)
  else(HAVE_ISINF_FUNCTION)
    check_symbol_exists(_isinf "math.h" HAVE__ISINF_SYMBOL)
    if(HAVE__ISINF_SYMBOL)
      set(PL__HAVE_ISINF ON)
    else(HAVE__ISINF_SYMBOL)
      check_function_exists(_isinf HAVE__ISINF_FUNCTION)
      if(HAVE__ISINF_FUNCTION)
        set(PL__HAVE_ISINF ON)
      endif(HAVE__ISINF_FUNCTION)
    endif(HAVE__ISINF_SYMBOL)
  endif(HAVE_ISINF_FUNCTION)
endif(HAVE_ISINF_SYMBOL)
if(PL__HAVE_ISINF)
  set(PL_HAVE_ISINF ON)
endif(PL__HAVE_ISINF)


check_symbol_exists(snprintf stdio.h PL_HAVE_SNPRINTF)
if(NOT PL_HAVE_SNPRINTF)
  check_symbol_exists(_snprintf stdio.h _PL_HAVE_SNPRINTF)
  set(PL_HAVE_SNPRINTF ${_PL_HAVE_SNPRINTF} CACHE INTERNAL "Have function _sprintf")
endif(NOT PL_HAVE_SNPRINTF)

# =======================================================================
# Language bindings
# =======================================================================

# Find swig.  Required for python, java and Lua bindings.
# N.B. all version tests done below need experimental FindSWIG.cmake which
# is currently carried in this directory by PLplot, but which eventually
# should get into CMake.
find_package(SWIG)
if(SWIG_FOUND)
  message(STATUS "SWIG_VERSION = ${SWIG_VERSION}")
  if(${SWIG_VERSION} VERSION_LESS "1.3.22")
    message(STATUS "WARNING: swig version too old.  SWIG_FOUND set to OFF")
    set(SWIG_FOUND OFF)
  endif(${SWIG_VERSION} VERSION_LESS "1.3.22")
endif(SWIG_FOUND)

if(SWIG_FOUND)
  if(${SWIG_VERSION} VERSION_LESS "1.3.30")
    set(SWIG_JAVA_NOPGCPP OFF)
  else(${SWIG_VERSION} VERSION_LESS "1.3.30")
    set(SWIG_JAVA_NOPGCPP ON)
  endif(${SWIG_VERSION} VERSION_LESS "1.3.30")
endif(SWIG_FOUND)

if(SWIG_FOUND)
  # Do not use "include(${SWIG_USE_FILE})" here since we want the option of
  # using a locally modified version of UseSWIG.cmake if that exists rather
  # than the official system version of that file.
  include(UseSWIG)
endif(SWIG_FOUND)

# Find Perl.  Required in several places in the build system (e.g.,
# tcl and docbook).  Also check for XML::Parser and XML::DOM which
# are also used several places in the build system.
find_package(Perl)
if(PERL_FOUND)
    include(CheckPerlModules)
    check_perl_modules(PERL_XML_PARSER XML::Parser)
    check_perl_modules(PERL_XML_DOM XML::DOM)
endif(PERL_FOUND)

# =======================================================================
# MATH_LIB must be defined (or not) before pkg-config.
# =======================================================================

# On windows systems the math library is not separated so do not specify
# it unless you are on a non-windows system.
if(NOT WIN32_OR_CYGWIN)
  find_library(MATH_LIB NAMES m PATHS /usr/local/lib /usr/lib)
  if(NOT MATH_LIB)
    message(FATAL_ERROR "Cannot find required math library")
  endif(NOT MATH_LIB)
endif(NOT WIN32_OR_CYGWIN)

# Temporary workaround for language support that is required
# for all language bindings.
include(language_support)

# C++ language support and binding
include(c++)

# =======================================================================
# pkg-config support as well as macros to put link flags in standard
# *.pc (pkg-config) form as well as standard fullpath form used by cmake.
# Must come after c++ is enabled (just above).
# =======================================================================
include(pkg-config)

# Find X headers, libraries, and library directory (required by xwin and
# cairo device drivers and also everything that is Tk related).
find_package(X11)

# Convert X11_LIBRARIES to full pathname in all cases (2.4 and 2.6).
cmake_link_flags(X11_LIBRARIES "${X11_LIBRARIES}")

if(X11_INCLUDE_DIR)
  # remove duplicates in the X11_INCLUDE_DIR list since those screw up
  # certain of our modules and also slow down compilation if any of
  # those duplicates leak through to the compile -I options.
  list(SORT X11_INCLUDE_DIR)
  set(copy_X11_INCLUDE_DIR ${X11_INCLUDE_DIR})
  set(listindex 0)
  set(listindexplus 1)
  foreach(copy_listelement ${copy_X11_INCLUDE_DIR})
    # need to get list elements corresponding to list indices.
    list(LENGTH X11_INCLUDE_DIR listlength)
    if(listindexplus LESS ${listlength})
      list(GET X11_INCLUDE_DIR ${listindex} listelement)
      list(GET X11_INCLUDE_DIR ${listindexplus} listelementplus)
    else(listindexplus LESS ${listlength})
      set(listelement)
      set(listelementplus)
    endif(listindexplus LESS ${listlength})
    if(copy_listelement STREQUAL "${listelement}" AND
    copy_listelement STREQUAL "${listelementplus}"
    )
      list(REMOVE_AT X11_INCLUDE_DIR ${listindex})
    else(copy_listelement STREQUAL "${listelement}" AND
    copy_listelement STREQUAL "${listelementplus}"
    )
      # update list indices
      math(EXPR listindex "${listindex} + 1")
      math(EXPR listindexplus "${listindex} + 1")
    endif(copy_listelement STREQUAL "${listelement}" AND
    copy_listelement STREQUAL "${listelementplus}"
    )
  endforeach(copy_listelement ${copy_X11_INCLUDE_DIR})
  string(REGEX REPLACE ";" " -I" X11_COMPILE_FLAGS "-I${X11_INCLUDE_DIR}")
endif(X11_INCLUDE_DIR)
message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_COMPILE_FLAGS = ${X11_COMPILE_FLAGS}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")

option(DEFAULT_NO_BINDINGS
"All language bindings are disabled by default"
OFF
)

# Individual language support (beyond C and C++)
include(fortran)
include(java)
include(python)
include(octave)
# tcl-related must come after c++ and fortran because of use of filter_rpath
include(tcl-related)
include(ada)
# OCaml support is included after the check for output drivers in order to
# check for extcairo support.
#include(ocaml)
include(lua)
include(d)

# =======================================================================
# Option to change to static run-time support for Microsoft C and
# C++ compilers.  This method should only work after those
# compilers are enabled (C is already enabled for the whole project
# and C++ should be enabled above).
# =======================================================================
if(WIN32_OR_CYGWIN AND NOT MINGW)
  # MinGW excluded because it does not enable a static runtime according
  # to http://lists-archives.com/mingw-users/00126-mingw-msvc-md-mt-ml.html .
  option(STATIC_RUNTIME "Set Windows non-MinGW compiler static runtime linkage if requested" OFF)
  if(STATIC_RUNTIME)
    set(flag_vars
      CMAKE_C_FLAGS
      CMAKE_C_FLAGS_DEBUG
      CMAKE_C_FLAGS_RELEASE
      CMAKE_C_FLAGS_MINSIZEREL
      CMAKE_C_FLAGS_RELWITHDEBINFO
      CMAKE_CXX_FLAGS
      CMAKE_CXX_FLAGS_DEBUG
      CMAKE_CXX_FLAGS_RELEASE
      CMAKE_CXX_FLAGS_MINSIZEREL
      CMAKE_CXX_FLAGS_RELWITHDEBINFO
      )

    foreach(flag_var ${flag_vars})
      if(${flag_var} MATCHES "/MD")
	string(REGEX REPLACE "/MD" "/MT" ${flag_var} "${${flag_var}}")
	set(${flag_var} "${${flag_var}}" CACHE STRING "Flags used by the compiler" FORCE)
      endif(${flag_var} MATCHES "/MD")
    endforeach(flag_var ${flag_vars})
  endif(STATIC_RUNTIME)
endif(WIN32_OR_CYGWIN AND NOT MINGW)

# =======================================================================
# additional library support
# =======================================================================
# shapelib must come after c++ and fortran because of use of filter_rpath
# Support for shapelib library for reading shapefile map data
include(shapelib)
include(freetype)
# csiro must come after MATH_LIB is defined (or not).
# csiro must come after c++ and fortran because of use of filter_rpath
include(csiro)


# =======================================================================
# libpango support
# =======================================================================
include(pango)

# =======================================================================
# Device drivers
# =======================================================================
# drivers must come after c++ and fortran because of use of filter_rpath
include(drivers)

# test_diff must come after drivers
include(test_diff)

# =======================================================================
# OCaml support (after drivers to check for extcairo)
# =======================================================================
include(ocaml)

# =======================================================================
# Miscellaneous other features - including docbook documentation
# =======================================================================
include(docbook)
include(doxygen)
include(summary)

# Calculate permissions that should be used for shared objects that
# are created by custom commands to be consistent with the permissions
# automatically used for shared objects created as targets.  The
# result (whether execute permissions are set) depends on
# CMAKE_INSTALL_SO_NO_EXE that is set in
# $prefix/share/cmake-2.8/Modules/Platform/Linux.cmake, where $prefix
# is the installation prefix for cmake itself, and the value depends
# on Linux platform.
if(CMAKE_INSTALL_SO_NO_EXE)
  # Permissions for shared objects are (rw-r--r--).
  set(SO_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    GROUP_READ
    WORLD_READ
    )
else(CMAKE_INSTALL_SO_NO_EXE)
  # Permissions for shared objects are (rwxr-xr-x).
  set(SO_PERMISSIONS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
    )
endif(CMAKE_INSTALL_SO_NO_EXE)



