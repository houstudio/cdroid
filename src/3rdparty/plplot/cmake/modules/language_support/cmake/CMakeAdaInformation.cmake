
#=============================================================================
# Copyright 2004-2011 Kitware, Inc.
# Copyright 2015 Alan W. Irwin
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

# This file sets the basic flags for the Ada language in CMake.
# It also loads the available platform file for the system-compiler
# if it exists.
# It also loads a system - compiler - processor (or target hardware)
# specific file, which is mainly useful for crosscompiling and embedded systems.

# some compilers use different extensions (e.g. sdcc uses .rel)
# so set the extension here first so it can be overridden by the compiler specific file

# Additional Ada comment:
# Contrary to C++ where this extension is either .o for Unix or .obj
# otherwise we have evidence on both MinGW (from years ago) and
# MinGW-w64 (now) that this extension is always .o for Ada regardless
# of platform.  So use that assumption unless or until we find a
# platform where that assumption is not correct (i.e., one where the
# error message "gnat1: incorrect object file extension" is emitted.)
set(CMAKE_Ada_OUTPUT_EXTENSION .o)

# According to Brad King, this change needed to adjust Ada language
# support to an internal change to language support infrastructure
# that occurred for CMake-2.6.0 and beyond.
set(CMAKE_Ada_OUTPUT_EXTENSION_REPLACE 1)

set(_INCLUDED_FILE 0)

# Load compiler-specific information.
if(CMAKE_Ada_COMPILER_ID)
  include(${CMAKE_Ada_MODULES}/Compiler/${CMAKE_Ada_COMPILER_ID}-Ada.cmake OPTIONAL)
endif()

set(CMAKE_BASE_NAME)
get_filename_component(CMAKE_BASE_NAME "${CMAKE_Ada_COMPILER}" NAME_WE)

# load the system- and compiler specific files
message(STATUS "CMAKE_Ada_COMPILER_ID = ${CMAKE_Ada_COMPILER_ID}")
if(CMAKE_Ada_COMPILER_ID)
  include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_Ada_COMPILER_ID}-Ada OPTIONAL RESULT_VARIABLE _INCLUDED_FILE)
endif()
if (NOT _INCLUDED_FILE)
  #disable because its irrelevant and might include something by accident.
#  include(Platform/${CMAKE_SYSTEM_NAME}-${CMAKE_BASE_NAME} OPTIONAL
#          RESULT_VARIABLE _INCLUDED_FILE)
endif ()
# We specify the compiler information in the system file for some
# platforms, but this language may not have been enabled when the file
# was first included.  Include it again to get the language info.
# Remove this when all compiler info is removed from system files.
if (NOT _INCLUDED_FILE)
  #disable because its irrelevant and might include something by accident.
#  include(Platform/${CMAKE_SYSTEM_NAME} OPTIONAL)
endif ()

if(CMAKE_Ada_COMPILER_LINKS_STATICALLY)
  set_property(GLOBAL PROPERTY TARGET_SUPPORTS_SHARED_LIBS FALSE)
endif()

# This should be included before the _INIT variables are
# used to initialize the cache.  Since the rule variables
# have if blocks on them, users can still define them here.
# But, it should still be after the platform file so changes can
# be made to those values.

if(CMAKE_USER_MAKE_RULES_OVERRIDE)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE "${_override}")
endif()

if(CMAKE_USER_MAKE_RULES_OVERRIDE_Ada)
  # Save the full path of the file so try_compile can use it.
  include(${CMAKE_USER_MAKE_RULES_OVERRIDE_Ada} RESULT_VARIABLE _override)
  set(CMAKE_USER_MAKE_RULES_OVERRIDE_Ada "${_override}")
endif()


# Create a set of shared library variable specific to Ada
# For 90% of the systems, these are the same flags as the C versions
# so if these are not set just copy the flags from the c version
if(NOT CMAKE_SHARED_LIBRARY_CREATE_Ada_FLAGS)
  set(CMAKE_SHARED_LIBRARY_CREATE_Ada_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_C_FLAGS})
endif()

if(NOT CMAKE_Ada_COMPILE_OPTIONS_PIC)
  set(CMAKE_Ada_COMPILE_OPTIONS_PIC ${CMAKE_C_COMPILE_OPTIONS_PIC})
endif()

if(NOT CMAKE_Ada_COMPILE_OPTIONS_PIE)
  set(CMAKE_Ada_COMPILE_OPTIONS_PIE ${CMAKE_C_COMPILE_OPTIONS_PIE})
endif()

if(NOT CMAKE_Ada_COMPILE_OPTIONS_DLL)
  set(CMAKE_Ada_COMPILE_OPTIONS_DLL ${CMAKE_C_COMPILE_OPTIONS_DLL})
endif()

if(NOT CMAKE_SHARED_LIBRARY_Ada_FLAGS)
  set(CMAKE_SHARED_LIBRARY_Ada_FLAGS ${CMAKE_SHARED_LIBRARY_C_FLAGS})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_LINK_Ada_FLAGS)
  set(CMAKE_SHARED_LIBRARY_LINK_Ada_FLAGS ${CMAKE_SHARED_LIBRARY_LINK_C_FLAGS})
endif()

if(NOT CMAKE_SHARED_LIBRARY_RUNTIME_Ada_FLAG)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_Ada_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG})
endif()

if(NOT CMAKE_SHARED_LIBRARY_RUNTIME_Ada_FLAG_SEP)
  set(CMAKE_SHARED_LIBRARY_RUNTIME_Ada_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_C_FLAG_SEP})
endif()

if(NOT CMAKE_SHARED_LIBRARY_RPATH_LINK_Ada_FLAG)
  set(CMAKE_SHARED_LIBRARY_RPATH_LINK_Ada_FLAG ${CMAKE_SHARED_LIBRARY_RPATH_LINK_C_FLAG})
endif()

if(NOT DEFINED CMAKE_EXE_EXPORTS_Ada_FLAG)
  set(CMAKE_EXE_EXPORTS_Ada_FLAG ${CMAKE_EXE_EXPORTS_C_FLAG})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_SONAME_Ada_FLAG)
  set(CMAKE_SHARED_LIBRARY_SONAME_Ada_FLAG ${CMAKE_SHARED_LIBRARY_SONAME_C_FLAG})
endif()

if(NOT CMAKE_EXECUTABLE_RUNTIME_Ada_FLAG)
  set(CMAKE_EXECUTABLE_RUNTIME_Ada_FLAG ${CMAKE_SHARED_LIBRARY_RUNTIME_Ada_FLAG})
endif()

if(NOT CMAKE_EXECUTABLE_RUNTIME_Ada_FLAG_SEP)
  set(CMAKE_EXECUTABLE_RUNTIME_Ada_FLAG_SEP ${CMAKE_SHARED_LIBRARY_RUNTIME_Ada_FLAG_SEP})
endif()

if(NOT CMAKE_EXECUTABLE_RPATH_LINK_Ada_FLAG)
  set(CMAKE_EXECUTABLE_RPATH_LINK_Ada_FLAG ${CMAKE_SHARED_LIBRARY_RPATH_LINK_Ada_FLAG})
endif()

if(NOT DEFINED CMAKE_SHARED_LIBRARY_LINK_Ada_WITH_RUNTIME_PATH)
  set(CMAKE_SHARED_LIBRARY_LINK_Ada_WITH_RUNTIME_PATH ${CMAKE_SHARED_LIBRARY_LINK_C_WITH_RUNTIME_PATH})
endif()

if(NOT CMAKE_INCLUDE_FLAG_Ada)
  set(CMAKE_INCLUDE_FLAG_Ada ${CMAKE_INCLUDE_FLAG_C})
endif()

if(NOT CMAKE_INCLUDE_FLAG_SEP_Ada)
  set(CMAKE_INCLUDE_FLAG_SEP_Ada ${CMAKE_INCLUDE_FLAG_SEP_C})
endif()

# for most systems a module is the same as a shared library
# so unless the variable CMAKE_MODULE_EXISTS is set just
# copy the values from the LIBRARY variables
if(NOT CMAKE_MODULE_EXISTS)
  set(CMAKE_SHARED_MODULE_Ada_FLAGS ${CMAKE_SHARED_LIBRARY_Ada_FLAGS})
  set(CMAKE_SHARED_MODULE_CREATE_Ada_FLAGS ${CMAKE_SHARED_LIBRARY_CREATE_Ada_FLAGS})
endif()

# repeat for modules
if(NOT CMAKE_SHARED_MODULE_CREATE_Ada_FLAGS)
  set(CMAKE_SHARED_MODULE_CREATE_Ada_FLAGS ${CMAKE_SHARED_MODULE_CREATE_C_FLAGS})
endif()

if(NOT CMAKE_SHARED_MODULE_Ada_FLAGS)
  set(CMAKE_SHARED_MODULE_Ada_FLAGS ${CMAKE_SHARED_MODULE_C_FLAGS})
endif()

# Initialize Ada link type selection flags from C versions.
foreach(type SHARED_LIBRARY SHARED_MODULE EXE)
  if(NOT CMAKE_${type}_LINK_STATIC_Ada_FLAGS)
    set(CMAKE_${type}_LINK_STATIC_Ada_FLAGS
      ${CMAKE_${type}_LINK_STATIC_C_FLAGS})
  endif()
  if(NOT CMAKE_${type}_LINK_DYNAMIC_Ada_FLAGS)
    set(CMAKE_${type}_LINK_DYNAMIC_Ada_FLAGS
      ${CMAKE_${type}_LINK_DYNAMIC_C_FLAGS})
  endif()
endforeach()

# add the flags to the cache based
# on the initial values computed in the platform/*.cmake files
# use _INIT variables so that this only happens the first time
# and you can set these flags in the cmake cache
set(CMAKE_Ada_FLAGS_INIT "$ENV{ADAFLAGS} ${CMAKE_Ada_FLAGS_INIT}")
# avoid just having a space as the initial value for the cache
if(CMAKE_Ada_FLAGS_INIT STREQUAL " ")
  set(CMAKE_Ada_FLAGS_INIT)
endif()
set (CMAKE_Ada_FLAGS "${CMAKE_Ada_FLAGS_INIT}" CACHE STRING
     "Flags used by the compiler during all build types.")

if(NOT CMAKE_NOT_USING_CONFIG_FLAGS)
  set (CMAKE_Ada_FLAGS_DEBUG "${CMAKE_Ada_FLAGS_DEBUG_INIT}" CACHE STRING
     "Flags used by the compiler during debug builds.")
  set (CMAKE_Ada_FLAGS_MINSIZEREL "${CMAKE_Ada_FLAGS_MINSIZEREL_INIT}" CACHE STRING
     "Flags used by the compiler during release builds for minimum size.")
  set (CMAKE_Ada_FLAGS_RELEASE "${CMAKE_Ada_FLAGS_RELEASE_INIT}" CACHE STRING
     "Flags used by the compiler during release builds.")
  set (CMAKE_Ada_FLAGS_RELWITHDEBINFO "${CMAKE_Ada_FLAGS_RELWITHDEBINFO_INIT}" CACHE STRING
     "Flags used by the compiler during release builds with debug info.")

endif()

if(CMAKE_Ada_STANDARD_LIBRARIES_INIT)
  set(CMAKE_Ada_STANDARD_LIBRARIES "${CMAKE_Ada_STANDARD_LIBRARIES_INIT}"
    CACHE STRING "Libraries linked by default with all Ada applications.")
  mark_as_advanced(CMAKE_Ada_STANDARD_LIBRARIES)
endif()

include(CMakeCommonLanguageInclude)

# now define the following rules:
# CMAKE_Ada_CREATE_SHARED_LIBRARY
# CMAKE_Ada_CREATE_SHARED_MODULE
# CMAKE_Ada_COMPILE_OBJECT
# CMAKE_Ada_LINK_EXECUTABLE

# variables supplied by the generator at use time
# <TARGET>
# <TARGET_BASE> the target without the suffix
# <OBJECTS>
# <OBJECT>
# <LINK_LIBRARIES>
# <FLAGS>
# <LINK_FLAGS>

# Ada compiler information
# <CMAKE_Ada_COMPILER>
# <CMAKE_SHARED_LIBRARY_CREATE_Ada_FLAGS>
# <CMAKE_SHARED_MODULE_CREATE_Ada_FLAGS>
# <CMAKE_Ada_LINK_FLAGS>

# Static library tools
# <CMAKE_AR>
# <CMAKE_RANLIB>

# create an Ada shared library
if(NOT CMAKE_Ada_CREATE_SHARED_LIBRARY)
  if(APPLE)
    # Ada adaptation of the special treatment of
    # CMAKE_CXX_CREATE_SHARED_LIBRARY in Modules/Platform/Darwin.cmake
    # where that special treatment has been invariant from CMake-3.0.2
    # to at least CMake-3.4.0.
    set(CMAKE_Ada_CREATE_SHARED_LIBRARY
      "<CMAKE_Ada_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_Ada_FLAGS> <LINK_FLAGS> -o <TARGET> <SONAME_FLAG> <TARGET_INSTALLNAME_DIR><TARGET_SONAME> <OBJECTS> <LINK_LIBRARIES>"
      )
  else(APPLE)
    # Follow CXX generic.
    set(CMAKE_Ada_CREATE_SHARED_LIBRARY
      "<CMAKE_Ada_COMPILER> <CMAKE_SHARED_LIBRARY_Ada_FLAGS> <LANGUAGE_COMPILE_FLAGS> <LINK_FLAGS> <CMAKE_SHARED_LIBRARY_CREATE_Ada_FLAGS> <CMAKE_SHARED_LIBRARY_SONAME_Ada_FLAG><TARGET_SONAME> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
      )
  endif(APPLE)
endif(NOT CMAKE_Ada_CREATE_SHARED_LIBRARY)

if(NOT CMAKE_Ada_CREATE_SHARED_MODULE)
  if(APPLE)
    # Ada adaptation of the special treatment of
    # CMAKE_CXX_CREATE_SHARED_MODULE in Modules/Platform/Darwin.cmake
    # where that special treatment has been invariant from CMake-3.0.2
    # to at least CMake-3.4.0.
    set(CMAKE_Ada_CREATE_SHARED_MODULE
      "<CMAKE_Ada_COMPILER> <LANGUAGE_COMPILE_FLAGS> <CMAKE_SHARED_MODULE_CREATE_Ada_FLAGS> <LINK_FLAGS> -o <TARGET> <OBJECTS> <LINK_LIBRARIES>"
      )
  else(APPLE)
    # Follow CXX generic which is to create an Ada shared module like an Ada shared library
    set(CMAKE_Ada_CREATE_SHARED_MODULE ${CMAKE_Ada_CREATE_SHARED_LIBRARY})
  endif(APPLE)
endif()

# create an Ada static library
if(NOT CMAKE_Ada_CREATE_STATIC_LIBRARY)
  set(CMAKE_Ada_CREATE_STATIC_LIBRARY
    "<CMAKE_AR> cr <TARGET> <LINK_FLAGS> <OBJECTS> "
    "<CMAKE_RANLIB> <TARGET> ")
endif(NOT CMAKE_Ada_CREATE_STATIC_LIBRARY)

# compile a Ada file into an object file
if(NOT CMAKE_Ada_COMPILE_OBJECT)
  if(NOT CMAKE_VERSION VERSION_LESS 3.4)
    set(CMAKE_Ada_COMPILE_OBJECT
    "<CMAKE_Ada_COMPILER>  <DEFINES> <INCLUDES> <FLAGS> -o <OBJECT> -c <SOURCE>")
  else()
    set(CMAKE_Ada_COMPILE_OBJECT
      "<CMAKE_Ada_COMPILER> <FLAGS> -c <SOURCE> -o <OBJECT>")
  endif()
endif()

# Constraints:  GNAT_EXECUTABLE_BUILDER = gnatmake
# is required to do the compile+bind+link of
# Ada executables, and it requires a source file name which is constructed from
# <TARGET>.adb.  The source file arguments of add_executable are
# all compiled by the above rule (which must remain that form since it
# is also used to compile objects for Ada libraries), but the results are
# ignored since they are put in a different directory while gnatmake assumes
# objects are located in the _current_ directory.  Thus, put in a minimal
# source file (with correct .adb suffix to identify the Ada language)
# to reduce this useless compilation to a minimum.  Usually, the main Ada
# routine qualifies since it is normally small.  Thus, the normal usage is
# add_executable(foo foo.adb), but  add_executable(foo path/minimal.adb) would
# work as well so long as both path/minimal.adb existed and foo.adb existed.
# Also, note there is no way to specify
# ${CMAKE_CURRENT_SOURCE_DIR}/<TARGET>.adb as the code for gnatmake to compile
# because in this context ${CMAKE_CURRENT_SOURCE_DIR} is set to the top
# of the source tree and not the expected sub-directory of the source tree.
# Thus, LINK_FLAGS -aI${CMAKE_CURRENT_SOURCE_DIR} must be set using
# set_target_properties in order to specify the directory where <TARGET>.adb
# exists. Note, LINK_FLAGS can also be used to set other gnatmake flags
# such as -aL.

# In sum, you have to be careful of your target name, the nominal source file
# name has to be compilable, but otherwise it is ignored, and you must specify
# the required -aI and other GNAT_EXECUTABLE_BUILDER = gnatmake options
# using LINK_FLAGS specified with set_target_properties.
# However, so long as you pay attention to these
# constraints, add_executable should work for the Ada language.

get_filename_component(COMPILER_LOCATION "${CMAKE_Ada_COMPILER}"
  PATH)
find_program(GNAT_EXECUTABLE_BUILDER NAMES gnatmake PATHS ${COMPILER_LOCATION} )

# From the above considerations CMAKE_Ada_LINK_EXECUTABLE needs to be in a special
# form for Ada.

# WARNING. For both the Platform/Cygwin-GNU.cmake and
# Platform/Windows-GNU.cmake cases (included indirectly above on the
# respective platforms), CMAKE_Ada_LINK_EXECUTABLE is set to
# "<CMAKE_${lang}_COMPILER> <FLAGS> <CMAKE_${lang}_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> -Wl,--out-implib,<TARGET_IMPLIB> ${CMAKE_GNULD_IMAGE_VERSION} <LINK_LIBRARIES>"
# This is the wrong form for Ada.  For example, it uses
# <CMAKE_${lang}_COMPILER> rather than the required
# ${GNAT_EXECUTABLE_BUILDER}.  Furthermore, the extra implib and
# image_version linker stuff seems suitable only for building
# components of libraries (as opposed to a standalone executable).  So
# ignore the CMAKE_Ada_LINK_EXECUTABLE value if it exists and use the
# following form on all platforms:
set(CMAKE_Ada_LINK_EXECUTABLE
  "${GNAT_EXECUTABLE_BUILDER} <CMAKE_Ada_LINK_FLAGS> <LINK_FLAGS> <TARGET_BASE>.adb -cargs <FLAGS> -largs <LINK_LIBRARIES>"
  )

mark_as_advanced(
CMAKE_Ada_FLAGS
CMAKE_Ada_FLAGS_DEBUG
CMAKE_Ada_FLAGS_MINSIZEREL
CMAKE_Ada_FLAGS_RELEASE
CMAKE_Ada_FLAGS_RELWITHDEBINFO
)
set(CMAKE_Ada_INFORMATION_LOADED 1)
