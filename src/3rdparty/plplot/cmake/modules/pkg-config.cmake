# cmake/modules/pkg-config.cmake
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

# Module for determining pkg-config configuration variables related to the
# install-tree build of the examples.
# Also create useful macros called pkg_check_pkgconfig to emulate the
# pkgconfig macro using the pkg_check_modules macro;
# pkg_config_link_flags to process CMake link flags into
# pkg-config standard form for the configured output *.pc files, and
# cmake_link_flags to process input link flags delivered
# by pkg-config into CMake standard form.

# The following variables are set:
# PKG_CONFIG_EXECUTABLE	  - name of pkg-config executable, but can also be
# 			    used for logical control with, e.g.,
# 			    if(PKG_CONFIG_EXECUTABLE)
# PKG_CONFIG_ENV	  - the string PKG_CONFIG_PATH=
#                           ${PKG_CONFIG_DIR}:$ENV{PKG_CONFIG_PATH} which
#  			    is used in example builds.
include(FindPkgConfig)

if(PKG_CONFIG_EXECUTABLE)
  message(STATUS "Looking for pkg-config - found")
  set(env_PKG_CONFIG_PATH $ENV{PKG_CONFIG_PATH})

  set(
    PKG_CONFIG_DIR
    ${CMAKE_INSTALL_PKG_CONFIG_DIR}
    CACHE PATH "PLplot install location for (architecture-independent) pkg-config files (*.pc)"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST PKG_CONFIG_DIR)

  # Will likely need WIN32 and CYGWIN stanzas as well
  if(MINGW)
    if(env_PKG_CONFIG_PATH)
      # On MinGW (and probably Cygwin and bare Windows) env_PKG_CONFIG_PATH
      # is semi-colon delimited.
      set(_pkg_config_path "${PKG_CONFIG_DIR};${env_PKG_CONFIG_PATH}")
    else(env_PKG_CONFIG_PATH)
      set(_pkg_config_path "${PKG_CONFIG_DIR}")
    endif(env_PKG_CONFIG_PATH)
    # Convert semi-colon delimited path string with colon drive
    # letters to a colong delimited path form with slash drive letters
    # that pkg-config configuration files can use.
    determine_msys_path(_pkg_config_path "${_pkg_config_path}")
  else(MINGW)
    # Only tested on Unix (Linux) so far where env_PKG_CONFIG_PATH
    # is colon-delimited.
    if(env_PKG_CONFIG_PATH)
      set(_pkg_config_path "${PKG_CONFIG_DIR}:${env_PKG_CONFIG_PATH}")
    else(env_PKG_CONFIG_PATH)
      set(_pkg_config_path "${PKG_CONFIG_DIR}")
    endif(env_PKG_CONFIG_PATH)
  endif(MINGW)

  set(PKG_CONFIG_ENV PKG_CONFIG_PATH="${_pkg_config_path}")

  # Figure out C++ special library support for traditional build system by
  # copying CMake's knowledge of those libraries (held in the CMake lists,
  # CMAKE_CXX_IMPLICIT_LINK_LIBRARIES and CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES)
  # for the C++ compiler chosen by the user.
  set(cxx_compiler_library_pathname_list)
  if(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
    foreach(cmake_cxx_implicit_link_library ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES})
      if(cmake_cxx_implicit_link_library)
	unset(library_pathname CACHE)
	find_library(library_pathname ${cmake_cxx_implicit_link_library} HINTS ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES})
	if(library_pathname)
	  list(APPEND cxx_compiler_library_pathname_list ${library_pathname})
	elseif(EXISTS ${cmake_cxx_implicit_link_library})
	  # Sometimes ${cmake_cxx_implicit_link_library} is the name of an existing file.
          # In this case assume it is the name of a library.
	  list(APPEND cxx_compiler_library_pathname_list ${cmake_cxx_implicit_link_library})
	else(library_pathname)
	  message(STATUS "WARNING: could not find special C++ library ${cmake_cxx_implicit_link_library} as an
   existing full pathname of a library or anywhere in CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES =
   ${CMAKE_CXX_IMPLICIT_LINK_DIRECTORIES}.
   So ${cmake_cxx_implicit_link_library} has been ignored in forming cxx_compiler_library_pathname_list")
	endif(library_pathname)
      endif(cmake_cxx_implicit_link_library)
    endforeach(cmake_cxx_implicit_link_library ${CMAKE_CXX_IMPLICIT_LINK_LIBRARIES})
  endif(CMAKE_CXX_IMPLICIT_LINK_LIBRARIES)
  # Some cmake versions (e.g., CMake-3.12.2) produce duplicate libraries in this
  # list, but don't remove them in case these are needed for static linking.
  # list(REMOVE_DUPLICATES cxx_compiler_library_pathname_list)
  message(STATUS "cxx_compiler_library_pathname_list = ${cxx_compiler_library_pathname_list}")

else(PKG_CONFIG_EXECUTABLE)
  message(STATUS "Looking for pkg-config - not found")
  message(STATUS
  "WARNING: Makefile+pkg-config version of examples build in the install tree will not work.")
endif(PKG_CONFIG_EXECUTABLE)

macro(pkg_check_pkgconfig _package _include_DIR _link_DIR _link_FLAGS _cflags _version)
  # Similar to legacy pkgconfig only these results are derived from
  # pkg_check_modules and therefore are returned as lists rather than
  # blank-delimited strings.  Also, the _link_FLAGS value is converted
  # to the list of full library paths preferred by CMake via the
  # cmake_link_flags macro.

  # N.B. if using this macro in more than one context, cache clashes will
  # occur unless optional trailing prefix argument is specified to distinguish
  # different contexts.
  set(_prefix ${ARGN})
  if(NOT _prefix)
    set(_prefix "_PKGCONFIG_TMP")
  endif(NOT _prefix)

  if(FORCE_EXTERNAL_STATIC)
    set(_xprefix ${_prefix}_STATIC)
  else(FORCE_EXTERNAL_STATIC)
    set(_xprefix ${_prefix})
  endif(FORCE_EXTERNAL_STATIC)

  pkg_check_modules(${_prefix} "${_package}")

  if(${_prefix}_FOUND)
    cmake_link_flags(${_link_FLAGS} "${${_xprefix}_LDFLAGS}")
    # If libraries cannot be not found, then that is equivalent to whole
    # pkg-config module not being found.
    if(NOT ${_link_FLAGS})
      set(${_prefix}_FOUND)
    endif(NOT ${_link_FLAGS})
  endif(${_prefix}_FOUND)

  if(${_prefix}_FOUND)
    set(${_include_DIR} ${${_xprefix}_INCLUDE_DIRS})
    set(${_link_DIR}    ${${_xprefix}_LIBRARY_DIRS})
    set(${_cflags}      ${${_xprefix}_CFLAGS})
    # N.B. must use ${_prefix}_VERSION} rather than ${_xprefix}_VERSION}
    # because the latter is not defined.
    set(${_version}     ${${_prefix}_VERSION})
    set(_return_VALUE 0)
  else(${_prefix}_FOUND)
    set(${_include_DIR})
    set(${_link_DIR})
    set(${_link_FLAGS})
    set(${_cflags})
    set(${_version})
    set(_return_VALUE 1)
  endif(${_prefix}_FOUND)
  #message("DEBUG: ${_prefix}_FOUND = ${${_prefix}_FOUND}")
  #message("DEBUG: ${_include_DIR} = ${${_include_DIR}}")
  #message("DEBUG: ${_link_DIR} = ${${_link_DIR}}")
  #message("DEBUG: ${_link_FLAGS} = ${${_link_FLAGS}}")
  #message("DEBUG: ${_cflags} = ${${_cflags}}")
  #message("DEBUG: ${_version} = ${${_version}}")
endmacro(pkg_check_pkgconfig)

function(pkg_config_link_flags link_flags_out link_flags_in)
  # Transform link flags into a form that is suitable to be used for
  # output pkg-config (*.pc) files.
  # N.B. ${link_flags_in} must be in quoted "${list_variable}" form
  # where list_variable is a CMake list.

  #message("(original link flags) = ${link_flags_in}")
  # First strip out optimized / debug options which are not needed
  # Currently only FindQt4 seems to need this.
  if(CMAKE_BUILD_TYPE MATCHES "Debug")
    # Get rid of the optimized keyword and immediately following library as
    # well as the debug keyword anywhere such patterns occur in the list.
    string(REGEX REPLACE "(^|;)optimized;[^;]*;" "\\1" link_flags "${link_flags_in}")
    string(REGEX REPLACE "(^|;)debug;" "\\1" link_flags "${link_flags}")
  else(CMAKE_BUILD_TYPE MATCHES "Debug")
    # Get rid of the debug keyword and immediately following library as
    # well as the optimized keyword anywhere such patterns occur in the list.
    string(REGEX REPLACE "(^|;)debug;[^;]*;" "\\1" link_flags "${link_flags_in}")
    string(REGEX REPLACE "(^|;)optimized;" "\\1" link_flags "${link_flags}")
  endif(CMAKE_BUILD_TYPE MATCHES "Debug")

  #message("(stripped link flags) = ${link_flags}")

  # Replace actual library names with the -LPATHNAME and -lLIBRARYNAME form
  # since it appears pkg-config handles that latter form much better (with
  # regard to keeping the correct order and eliminating duplicates).

  # These suffix patterns for library pathnames appear to work fine on
  # Linux, Mac OS X, and MinGW/MSYS but it may need some
  # generalization on other platforms such as Cygwin.

  if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    set(suffix_pattern "(\\.so.*|\\.a)")
  elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    set(suffix_pattern "(\\.so.*|\\.a|\\.dylib)")
  elseif(WIN32_OR_CYGWIN)
    # Order is important here.
    set(suffix_pattern "(\\.dll\\.a|\\.a)")
  else(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    # Probably a non-Linux, non-Mac OS X, Unix platform
    # For this case we assume the same as Linux.
    set(suffix_pattern "(\\.so.*|\\.a)")
  endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")

  set(_link_flags)
  foreach(link_flag IN LISTS link_flags)
    #message(STATUS "link_flag = ${link_flag}")
    if(WIN32_OR_CYGWIN)
      # Look for colon-delimited drive-letter form on these platforms.
      string(REGEX REPLACE "^([a-zA-Z]:/.*)/lib(.*)${suffix_pattern}$" "-L\"\\1\" -l\\2" link_flag "${link_flag}")
    endif(WIN32_OR_CYGWIN)
    # Look for form starting with "/" on all platforms.
    string(REGEX REPLACE "^(/.*)/lib(.*)${suffix_pattern}$" "-L\"\\1\" -l\\2" link_flag "${link_flag}")
    #message(STATUS "(-L form of link_flag = ${link_flag}")
    list(APPEND _link_flags ${link_flag})
  endforeach(link_flag IN LISTS link_flags)
  set(link_flags ${_link_flags})
  set(_link_flags)
  #message("(-L form of link_flags) = ${link_flags}")

  # Convert link flags to a blank-delimited string.
  string(REGEX REPLACE ";" " " link_flags "${link_flags}")
  #message("(blank-delimited) link_flags = ${link_flags}")

  if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    # For Mac OS X transform frameworks information into correct form.
    string(
    REGEX REPLACE
    "/System/Library/Frameworks/([^ ]*)\\.framework"
    "-framework \\1"
    link_flags
    "${link_flags}"
    )
    #message("(frameworks) link_flags = ${link_flags}")
  endif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")

  set(${link_flags_out} ${link_flags} PARENT_SCOPE)

endfunction(pkg_config_link_flags)

macro(cmake_link_flags _link_flags_out _link_flags_in)
  # Transform link flags delivered by pkg-config into the best form
  # for CMake.
  # N.B. may need revision for windows since the current assumption
  # is pkg-config delivers link flags using the -L and -l options which
  # may not be the case for windows.
  # N.B. ${_link_flags_in} must be a string and not a list.

  if("${_link_flags_in}" STREQUAL "")
    set(${_link_flags_out})
  else("${_link_flags_in}" STREQUAL "")
    #message("(original link flags) = ${_link_flags_in}")
    # Convert link flags to a list.  Note some link flags are blank-delimited
    # (such as "-framework whatever") so preserve those by splitting into
    # separate list elements only if the next element starts with a hyphen.
    string(REGEX REPLACE " -" ";-" _link_flags_list "${_link_flags_in}")
    if(NOT MATH_LIB)
      # On systems (normally Windows) where MATH_LIB is not defined
      # filter out -lm from link flags.  This works around a pkg-config
      # configuration bug on Windows for the GTK+ stack of libraries where
      # -lm is supplied as a required library flag when that library does
      # not exist on Windows platforms.
      list(REMOVE_ITEM _link_flags_list "-lm")
    endif(NOT MATH_LIB)
    #message("_link_flags_list = ${_link_flags_list}")

    # Extract list of directories from -L options.
    list(LENGTH _link_flags_list _link_flags_length)
    math(EXPR _link_flags_length "${_link_flags_length} - 1")
    set(_index_list)
    set(_link_directory_list)
    foreach(_list_index RANGE ${_link_flags_length})
      list(GET _link_flags_list ${_list_index} _list_element)
      string(REGEX REPLACE "^-L" "" _list_element1 "${_list_element}")
      if(_list_element STREQUAL "-L${_list_element1}")
        list(APPEND _index_list ${_list_index})
        list(APPEND _link_directory_list ${_list_element1})
      endif(_list_element STREQUAL "-L${_list_element1}")
    endforeach(_list_index RANGE ${_link_flags_length})
    #message("_index_list = ${_index_list}")
    if(NOT "${_index_list}" STREQUAL "")
      # Remove -L options from list.
      list(REMOVE_AT _link_flags_list ${_index_list})
    endif(NOT "${_index_list}" STREQUAL "")
    #message("_link_directory_list = ${_link_directory_list}")
    #message("_link_flags_list (without -L options) = ${_link_flags_list}")

    # Derive ${_link_flags_out} from _link_flags_list with -l options
    # replaced by complete pathname of library.
    list(LENGTH _link_flags_list _link_flags_length)
    math(EXPR _link_flags_length "${_link_flags_length} - 1")
    set(${_link_flags_out})
    set(_success ON)
    set(_not_found_list)
    foreach(_list_index RANGE ${_link_flags_length})
      list(GET _link_flags_list ${_list_index} _list_element)
      string(REGEX REPLACE "^-l" "" _list_element1 "${_list_element}")
      if(_list_element STREQUAL "-l${_list_element1}")
        set(_library_pathname "_library_pathname-NOTFOUND")
        find_library(
         _library_pathname
         ${_list_element1}
         PATHS ${_link_directory_list}
         NO_DEFAULT_PATH
        )
	# Try second time (without NO_DEFAULT_PATH) just in case pkg-config
	# specified some system libraries without corresponding -L options.
        find_library(
         _library_pathname
         ${_list_element1}
         PATHS ${_link_directory_list}
        )
        if(NOT _library_pathname)
	  set(_success OFF)
	  list(APPEND _not_found_list ${_list_element1})
        endif(NOT _library_pathname)
        list(APPEND ${_link_flags_out} ${_library_pathname})
      else(_list_element STREQUAL "-L${_list_element1}")
        # link options that are not -L or -l passed through in correct order
        # in ${_link_flags_out}.
        list(APPEND ${_link_flags_out} ${_list_element})
      endif(_list_element STREQUAL "-l${_list_element1}")
    endforeach(_list_index RANGE ${_link_flags_length})
    #message("${_link_flags_out} = ${${_link_flags_out}}")

    # If one or more of the libraries cannot be found, then return an
    # empty ${_link_flags_out} as a sign of that failure.
    if(NOT _success)
      message(STATUS "WARNING: Library inconsistency detected.  Your pkg-config files claim certain libraries are present, but CMake cannot find them.")
      message(STATUS "The unfound library basenames are as follows:")
      foreach(_not_found IN LISTS _not_found_list)
	message(STATUS "${_not_found}")
      endforeach(_not_found IN LISTS _not_found_list)
      message(STATUS "Some library installation and/or adjustment of find-related environment variables or corresponding CMake variables will be required before CMake will be able to find these libraries.")
      message(STATUS "cmake_link_flags WARNING: (original link flags) = ${_link_flags_in}")
      message(STATUS "cmake_link_flags WARNING: (corresponding link flags with found libraries = ${_link_flags_out}) = ${${_link_flags_out}}")
      message(STATUS "cmake_link_flags WARNING: ${_link_flags_out} is invalid so it is set to nothing to signal the failure of cmake_link_flags for the original link flags printed out above.")
      set(${_link_flags_out})
    endif(NOT _success)
  endif("${_link_flags_in}" STREQUAL "")
endmacro(cmake_link_flags)

function(pkg_config_file BINDING PC_SHORT_NAME PC_LONG_NAME PC_LIBRARY_NAME PC_COMPILE_FLAGS PC_LINK_FLAGS)
  # Configure and install pkg-config *.pc file corresponding to the core PLplot library
  # or one of the language bindings libraries which depend on that core library.
  # The arguments are the following:
  # BINDING 	      	  - ENABLE_${BINDING} keeps track of whether a
  # 			    binding has been enabled (ON) or not (OFF).
  #			    Also, ${BINDING} used to determine PC_FILE_SUFFIX
  #			    which helps to determine name of configured
  #			    *.pc file.
  # PC_SHORT_NAME	  - Used in *.pc NAME: field
  # PC_LONG_NAME	  - Used in *.pc Description: field.  Must have trailing ", "
  #                         if you want additional precision information appended.
  # PC_LIBRARY_NAME	  - Used in *.pc Libs: field
  #                         Also used to determine PC_LINK_FLAGS and
  #                         PC_COMPILE_FLAGS used in *.pc Libs: and Cflags:
  #			    fields.
  # PC_COMPILE_FLAGS      - Space-delimited string of compile flags for this library.
  # PC_LINK_Flags         - List of libraries that this library depends on.  These will
  #                         be added to the public libraries for NON_TRANSITIVE OFF
  #                         and added to the private libraries for NON_TRANSITIVE ON
  #
  # There may be one additional optional argument (accessed with ${ARGN}) containing
  # a list of additional libraries to be added to the public libraries regardless of
  # NON_TRANSITIVE value.

  # An example of a call of this function using these arguments is
  # pkg_config_file("tcl" "Tcl/Tk" "Tcl/Tk bindings, " "plplottcltk" "${libplplottcltk_COMPILE_FLAGS}" "${libplplottcltk_LINK_FLAGS}")

  # This function depends on the following non-argument CMake variables being set.
  # PKG_CONFIG_EXECUTABLE
  # PL_DOUBLE
  # NON_TRANSITIVE
  # PKG_CONFIG_DIR
  # These last non-argument CMake variables required for configuring
  # pkgcfg/plplot-template.pc.in
  # PLPLOT_VERSION
  # INCLUDE_DIR
  # LIB_DIR

  if(PKG_CONFIG_EXECUTABLE)
    #message(STATUS "pkg_config_file called with ARGV = ${ARGV}")

    if(PC_LONG_NAME MATCHES "^ ")
      if(PL_DOUBLE)
	set(PC_PRECISION "Double precision")
      else(PL_DOUBLE)
	set(PC_PRECISION "Single precision")
      endif(PL_DOUBLE)
    else(PC_LONG_NAME MATCHES "^ ")
      set(PC_PRECISION)
    endif(PC_LONG_NAME MATCHES "^ ")

    # Filter all build-tree or source-tree -I options from
    # compile-flags.  These occur, for example, for libplplot in the
    # -DENABLE_DYNDRIVERS=OFF case because the compile flags of
    # certain device drivers (e.g., tk) depend on build-tree
    # directories, but libraries or applications that are built
    # against libplplot should _never_ include those build-tree
    # headers.
    string(REGEX REPLACE "-I[^ ]*${PROJECT_SOURCE_DIR}[^ ]*" "" PC_COMPILE_FLAGS "${PC_COMPILE_FLAGS}")
    string(REGEX REPLACE "-I[^ ]*${PROJECT_BINARY_DIR}[^ ]*" "" PC_COMPILE_FLAGS "${PC_COMPILE_FLAGS}")
    # Clean up extra blanks
    string(REGEX REPLACE "  *" " " PC_COMPILE_FLAGS "${PC_COMPILE_FLAGS}")

    # Transform PC_LINK_FLAGS from list of libraries to the standard pkg-config form.
    #message(STATUS "input PC_LINK_FLAGS = ${PC_LINK_FLAGS}")
    pkg_config_link_flags(PC_LINK_FLAGS "${PC_LINK_FLAGS}")
    #message(STATUS "pkg-config form of PC_LINK_FLAGS = ${PC_LINK_FLAGS}")
    if(NON_TRANSITIVE)
      set(PC_PRIVATE_LINK_FLAGS ${PC_LINK_FLAGS})
    else(NON_TRANSITIVE)
      set(PC_PUBLIC_LINK_FLAGS ${PC_LINK_FLAGS})
    endif(NON_TRANSITIVE)

    if(ARGC EQUAL 7)
      #message(STATUS "pkg_config_file called with ARGN = ${ARGN}")
      pkg_config_link_flags(added_private_libraries "${ARGN}")
      set(PC_PUBLIC_LINK_FLAGS "${PC_PUBLIC_LINK_FLAGS} ${added_private_libraries}")
    elseif(ARGC LESS 6)
      message(STATUS "pkg_config_file called with ARGV = ${ARGV}")
      message(FATAL_ERROR "pkg_config_file called with wrong number of arguments")
    endif(ARGC EQUAL 7)

    # N.B. all the tests on "X${BINDING}X" below rather than BINDING
    # are to beat a potential dereferencing problem for strings in if statements.
    # This problem only occurs with old CMake versions that don't
    # have CMP0054 set to NEW.

    if("X${BINDING}X" STREQUAL "XcX")
      set(PC_FILE_SUFFIX "")
      set(PC_REQUIRES "")
    elseif("X${BINDING}X" STREQUAL "XwxwidgetsX")
      set(PC_FILE_SUFFIX "-${BINDING}")
      set(PC_REQUIRES "plplot-c++")
    else("X${BINDING}X" STREQUAL "XcX")
      set(PC_FILE_SUFFIX "-${BINDING}")
      set(PC_REQUIRES "plplot")
    endif("X${BINDING}X" STREQUAL "XcX")

    if(NON_TRANSITIVE)
      if("X${BINDING}X" STREQUAL "XocamlX")
	# Don't know how to do non-transitive linking for
        # Ocaml binding of PLplot.
	set(PC_REQUIRES_TAG "Requires")
      else("X${BINDING}X" STREQUAL "XocamlX")
	set(PC_REQUIRES_TAG "Requires.private")
      endif("X${BINDING}X" STREQUAL "XocamlX")
    else(NON_TRANSITIVE)
      set(PC_REQUIRES_TAG "Requires")
    endif(NON_TRANSITIVE)

    # Include library itself in space-separated list of public libraries for
    # this package.
    set(PC_PUBLIC_LINK_FLAGS "-l${PC_LIBRARY_NAME} ${PC_PUBLIC_LINK_FLAGS}")

    set(PC_CONFIGURED_FILE
      ${PROJECT_BINARY_DIR}/pkgcfg/plplot${PC_FILE_SUFFIX}.pc
      )
    configure_file(
      ${PROJECT_SOURCE_DIR}/pkgcfg/plplot-template.pc.in
      ${PC_CONFIGURED_FILE}
      @ONLY
      )
    install(FILES ${PC_CONFIGURED_FILE} DESTINATION ${PKG_CONFIG_DIR})

  endif(PKG_CONFIG_EXECUTABLE)
endfunction(pkg_config_file)
