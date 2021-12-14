# cmake/modules/tcl-related.cmake
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

# Module for determining all tcl-related configuration options not directly
# related to the tk device driver.

if(DEFAULT_NO_BINDINGS)
  option(ENABLE_tcl "Enable Tcl binding" OFF)
  option(ENABLE_itcl "Enable Itcl interface code" OFF)
  option(ENABLE_tk "Enable Tk interface code" OFF)
  option(ENABLE_itk "Enable Itk interface code" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_tcl "Enable Tcl binding" ON)
  option(ENABLE_itcl "Enable Itcl interface code" ON)
  option(ENABLE_tk "Enable Tk interface code" ON)
  option(ENABLE_itk "Enable Itk interface code" ON)
endif(DEFAULT_NO_BINDINGS)

option(USE_INCRTCL_VERSION_4 "Use version 4 of Itcl and Itcl, version 4.1 of Iwidgets" OFF)

option(USE_TCL_TK_STUBS "Use Tcl/Tk stubs libraries" OFF)

# Depending on these above options and system resources may also determine
# the following variables which are largely self-explanatory unless documented
# further.

#TCL_INCLUDE_PATH
#TCL_LIBRARY             (Always the non-stub version)
#TCL_STUB_LIBRARY        (Always the stub version)
#ITCL_INCLUDE_PATH
#ITCL_LIBRARY
#HAVE_ITCL               (On when itcl header and library have been found.
#			  Otherwise, undefined.
#			  Used for source file configuration.)
#HAVE_ITCLDECLS_H 	 (defined [actually as a path, but we don't use that]
#			  when itclDecls.h.  Otherwise, undefined.
#			  Used for source file configuration.)
#TK_INCLUDE_PATH
#TK_LIBRARY              (Always the non-stub version)
#TK_STUB_LIBRARY         (Always the stub version)
#ITK_INCLUDE_PATH
#ITK_LIBRARY
#HAVE_ITK   		 (On when itcl header and library have been found.
#			  Otherwise, undefined.
#			  Used for source file configuration.)
#PLPLOT_ITCL_VERSION     (Consistent Itcl version number found by PLplot).
#PLPLOT_ITK_VERSION      (Consistent Itk version number found by PLplot).
#PLPLOT_IWIDGETS_VERSION (Consistent Iwidgets version number found by PLplot).
#TCL_RPATH               (rpath, if needed for non-system Tcl location)
#TCL_TK_RPATH            (rpath, if needed for non-system Tcl/Tk location)
#TCL_TK_ITCL_ITK_RPATH   (rpath, if needed for non-system Tcl/Tk/Itcl/Itk
#                         location)

if(ENABLE_tcl)
  message(STATUS "Start determining consistent system data for Tcl and friends")
  find_package(TclStub QUIET)
  if(TCL_FOUND)
    message(STATUS "Looking for Tcl - found")
    message(STATUS "TCL_INCLUDE_PATH = ${TCL_INCLUDE_PATH}")
    message(STATUS "TCL_LIBRARY = ${TCL_LIBRARY}")
    message(STATUS "TCL_STUB_LIBRARY = ${TCL_STUB_LIBRARY}")

    get_filename_component(TCL_LIBRARY_PATH ${TCL_LIBRARY} PATH)
    message(STATUS "TCL_LIBRARY_PATH = ${TCL_LIBRARY_PATH}")

    if(TCL_TCLSH)
      message(STATUS "Looking for tclsh - found")
      message(STATUS "TCL_TCLSH = ${TCL_TCLSH}")
      if(NOT PLPLOT_TCL_VERSION)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CheckTCL_VERSION.tcl "puts -nonewline [package require Tcl]; exit")
	# Find Tcl version via tclsh or fail.
	execute_process(
	  COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_BINARY_DIR}/CheckTCL_VERSION.tcl
	  RESULT_VARIABLE TCL_RC
	  OUTPUT_VARIABLE _plplot_tcl_version
	  )
	if(NOT TCL_RC)
	  message(STATUS "Looking for Tcl version with tclsh - found")
	  # Store result in cached variable so it will be remembered
          # when cmake is re-run, but use type of STRING rather than
          # INTERNAL to allow users to specify the value with a
          # -DPLPLOT_TCL_VERSION:STRING=<whatever> on the command
          # line for those rare cases when "package require Tcl"
          # would error out due to some tclsh issue.
	  set(PLPLOT_TCL_VERSION ${_plplot_tcl_version}
	    CACHE STRING "Tcl version that is available"
	    )
	else(NOT TCL_RC)
	  message(STATUS "Looking for Tcl version with tclsh - not found")
	  message(STATUS "WARNING: setting ENABLE_tcl to OFF")
	  set(ENABLE_tcl OFF CACHE BOOL "Enable Tcl binding" FORCE)
	endif(NOT TCL_RC)
      endif(NOT PLPLOT_TCL_VERSION)
    else(TCL_TCLSH)
      message(STATUS "Looking for tclsh - not found")
      message(STATUS "WARNING: setting ENABLE_tcl to OFF")
      set(ENABLE_tcl OFF CACHE BOOL "Enable Tcl binding" FORCE)
    endif(TCL_TCLSH)
  else(TCL_FOUND)
    message(STATUS "Looking for Tcl - not found")
    message(STATUS "WARNING: setting ENABLE_tcl to OFF")
    set(ENABLE_tcl OFF CACHE BOOL "Enable Tcl binding" FORCE)
  endif(TCL_FOUND)
endif(ENABLE_tcl)

if(ENABLE_tcl)
  message(STATUS "PLPLOT_TCL_VERSION = ${PLPLOT_TCL_VERSION}")

  set(TCL_RPATH ${TCL_LIBRARY_PATH})
  set(TCL_TK_RPATH ${TCL_RPATH})
  set(TCL_TK_ITCL_ITK_RPATH ${TCL_RPATH})

  # Sanity checking of Tcl version consistency _if_ a Tcl version string
  # can be extracted from the Tcl library name.
  get_filename_component(library_name ${TCL_LIBRARY} NAME)
  string(REGEX MATCH "[0-9]+[.]?[0-9]+" library_version ${library_name})
  if(library_version)
    # If no decimal point in version insert as the second character.
    if(NOT library_version MATCHES "[.]")
      string(REGEX REPLACE "^([0-9])([0-9]+)$" "\\1.\\2" library_version ${library_version})
    endif(NOT library_version MATCHES "[.]")
    #message(STATUS "library_version = ${library_version}")
    string(LENGTH ${library_version} length_library_version)
    string(SUBSTRING ${PLPLOT_TCL_VERSION} 0 ${length_library_version} truncated_version)
    #message(STATUS "truncated_version = ${truncated_version}")
    if(NOT truncated_version STREQUAL library_version)
      message(STATUS "WARNING: Tcl library version = ${library_version} that is extracted from the
   library name is not consistent with PLPLOT_TCL_VERSION = ${PLPLOT_TCL_VERSION}")
    endif(NOT truncated_version STREQUAL library_version)
  endif(library_version)

  if(ENABLE_itcl)
    if(USE_INCRTCL_VERSION_4)
      set(SUGGESTED_ITCL_VERSION 4)
    else(USE_INCRTCL_VERSION_4)
      set(SUGGESTED_ITCL_VERSION 3)
    endif(USE_INCRTCL_VERSION_4)
    if(NOT PLPLOT_ITCL_VERSION)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CheckITCL_Available.tcl "puts -nonewline [package require Itcl ${SUGGESTED_ITCL_VERSION}]; exit")
      # Refine SUGGESTED_ITCL_VERSION to exact value or fail.
      execute_process(
	COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_BINARY_DIR}/CheckITCL_Available.tcl
	RESULT_VARIABLE ITCL_RC
	OUTPUT_VARIABLE _plplot_itcl_version
	)
      if(NOT ITCL_RC)
	# Store result in cached variable so it will be remembered
        # when cmake is re-run, but use type of STRING rather than
        # INTERNAL to allow users to specify the value with a
        # -DPLPLOT_ITCL_VERSION:STRING=<whatever> on the command
        # line for those rare cases when "package require Itcl"
        # would error out due to some tclsh issue.
	set(PLPLOT_ITCL_VERSION ${_plplot_itcl_version}
	  CACHE STRING "Itcl version that will be used"
	  )
      endif(NOT ITCL_RC)
    else(NOT PLPLOT_ITCL_VERSION)
      set(ITCL_RC 0)
    endif(NOT PLPLOT_ITCL_VERSION)

    if(NOT ITCL_RC)
      message(STATUS "Looking for itcl.h")
      if(NOT USE_INCRTCL_VERSION_4)
	# Search first for epa_build install location.
        find_path(ITCL_INCLUDE_PATH itcl.h HINTS "${TCL_INCLUDE_PATH}/itcl${PLPLOT_ITCL_VERSION}")
      endif(NOT USE_INCRTCL_VERSION_4)
      message(STATUS "PLPLOT_ITCL_VERSION = ${PLPLOT_ITCL_VERSION}")
      string(SUBSTRING ${PLPLOT_ITCL_VERSION} 0 1 SHORT_PLPLOT_ITCL_VERSION)
      # N.B. Debian jessie itcl3-dev package stores itcl.h in /usr/include/itcl${SHORT_PLPLOT_ITCL_VERSION}
      find_path(ITCL_INCLUDE_PATH itcl.h HINTS "${TCL_INCLUDE_PATH}" PATH_SUFFIXES itcl${SHORT_PLPLOT_ITCL_VERSION})

      if(ITCL_INCLUDE_PATH)
        message(STATUS "Looking for itcl.h - found")
	message(STATUS "ITCL_INCLUDE_PATH = ${ITCL_INCLUDE_PATH}")

	message(STATUS "Checking for Itcl header version consistency")
	string(REGEX REPLACE "^([0-9]*)\\.[0-9]*.*$" "\\1" ITCL_MAJOR_VERSION  "${PLPLOT_ITCL_VERSION}")
	string(REGEX REPLACE "^[0-9]*\\.([0-9]*).*$" "\\1" ITCL_MINOR_VERSION  "${PLPLOT_ITCL_VERSION}")
	message(STATUS "ITCL_MAJOR_VERSION = ${ITCL_MAJOR_VERSION}")
	message(STATUS "ITCL_MINOR_VERSION = ${ITCL_MINOR_VERSION}")
	if(NOT DEFINED CONSISTENT_HEADER_ITCL_VERSION)
	  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CheckITCL_VERSION.c
	    "// #define RC_INVOKED to avoid sucking in lots of additional includes for
// efficiency and simplicity.
#define RC_INVOKED 1
#include <itcl.h>
#if ITCL_MAJOR_VERSION != ${ITCL_MAJOR_VERSION} || ITCL_MINOR_VERSION != ${ITCL_MINOR_VERSION}
  #error ITCL_VERSION mismatch
#endif
void main(void){}
"
	    )
	  try_compile(CONSISTENT_HEADER_ITCL_VERSION ${PROJECT_BINARY_DIR}
	    ${CMAKE_CURRENT_BINARY_DIR}/CheckITCL_VERSION.c
	    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:STRING=${ITCL_INCLUDE_PATH};${TCL_INCLUDE_PATH}"
	    )
	endif(NOT DEFINED CONSISTENT_HEADER_ITCL_VERSION)

	if(CONSISTENT_HEADER_ITCL_VERSION)
	  message(STATUS "Checking for Itcl header version consistency - true")
	else(CONSISTENT_HEADER_ITCL_VERSION)
	  message(STATUS "Checking for Itcl header version consistency - false")
          message(STATUS "WARNING: Disabling Itcl interface code")
	  set(ENABLE_itcl OFF CACHE BOOL "Enable Itcl interface code" FORCE)
	endif(CONSISTENT_HEADER_ITCL_VERSION)
      else(ITCL_INCLUDE_PATH)
        message(STATUS "Looking for itcl.h - not found")
        message(STATUS "WARNING: Disabling Itcl interface code")
        set(ENABLE_itcl OFF CACHE BOOL "Enable Itcl interface code" FORCE)
      endif(ITCL_INCLUDE_PATH)

      if(ENABLE_itcl)
        message(STATUS "Looking for itcl library")
	# Search first for epa_build install location.
	string(REGEX REPLACE "\\." "" COMPACT_PLPLOT_ITCL_VERSION ${PLPLOT_ITCL_VERSION})
        find_library(ITCL_LIBRARY NAMES itcl${PLPLOT_ITCL_VERSION} itcl${COMPACT_PLPLOT_ITCL_VERSION} itcl HINTS ${TCL_LIBRARY_PATH}/itcl${PLPLOT_ITCL_VERSION})
        find_library(ITCL_LIBRARY NAMES itcl${PLPLOT_ITCL_VERSION} itcl${COMPACT_PLPLOT_ITCL_VERSION} itcl HINTS ${TCL_LIBRARY_PATH})
        if(ITCL_LIBRARY)
          message(STATUS "Looking for itcl library - found")
	  message(STATUS "ITCL_LIBRARY = ${ITCL_LIBRARY}")
	  get_filename_component(ITCL_LIBRARY_PATH ${ITCL_LIBRARY} PATH)
	  list(APPEND TCL_TK_ITCL_ITK_RPATH ${ITCL_LIBRARY_PATH})
        else(ITCL_LIBRARY)
          message(STATUS "Looking for itcl library - not found")
          message(STATUS "WARNING: Disabling Itcl interface code")
          set(ENABLE_itcl OFF CACHE BOOL "Enable Itcl interface code" FORCE)
        endif(ITCL_LIBRARY)
      endif(ENABLE_itcl)
    else(NOT ITCL_RC)
      message(STATUS "Itcl not available or not compatible with current Tcl shell")
      message(STATUS "WARNING: Disabling Itcl interface code")
      set(ENABLE_itcl OFF CACHE BOOL "Enable Itcl interface code" FORCE)
    endif(NOT ITCL_RC)
  endif(ENABLE_itcl)

  if(ENABLE_tk AND TK_FOUND)
    message(STATUS "Looking for Tk - found")
    if(X11_FOUND)
      list(APPEND TK_INCLUDE_PATH ${X11_INCLUDE_DIR})
    endif(X11_FOUND)
    message(STATUS "TK_INCLUDE_PATH = ${TK_INCLUDE_PATH}")
    message(STATUS "TK_LIBRARY = ${TK_LIBRARY}")
    message(STATUS "TK_STUB_LIBRARY = ${TK_STUB_LIBRARY}")

    message(STATUS "TK_LIBRARY_PATH = ${TK_LIBRARY_PATH}")
    if(NOT ${TK_LIBRARY_PATH} STREQUAL ${TCL_LIBRARY_PATH})
      message(STATUS "WARNING: the Tcl and Tk library locations are inconsistent so those libraries are likely not compatible")
    endif(NOT ${TK_LIBRARY_PATH} STREQUAL ${TCL_LIBRARY_PATH})

    if(TK_WISH)
      message(STATUS "Looking for wish - found")
      message(STATUS "TK_WISH = ${TK_WISH}")
      if(NOT PLPLOT_TK_VERSION)
	file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CheckTK_VERSION.tcl "puts -nonewline [package require Tk]; exit")
	# Find Tk version via wish or fail.
	execute_process(
	  COMMAND ${TK_WISH} ${CMAKE_CURRENT_BINARY_DIR}/CheckTK_VERSION.tcl
	  RESULT_VARIABLE TK_RC
	  OUTPUT_VARIABLE _plplot_tk_version
	  )
	if(NOT TK_RC)
	  message(STATUS "Looking for Tk version with wish - found")
	  # Store result in cached variable so it will be remembered
          # when cmake is re-run, but use type of STRING rather than
          # INTERNAL to allow users to specify the value with a
          # -DPLPLOT_TK_VERSION:STRING=<whatever> on the command
          # line for those rare cases when "package require Tk"
          # would error out due to some wish issue.
	  set(PLPLOT_TK_VERSION ${_plplot_tk_version}
	    CACHE STRING "Tk version found by wish"
	    )

	  if(PLPLOT_TCL_VERSION STREQUAL PLPLOT_TK_VERSION)
	    # Cross check that wish finds the same Tcl version as tclsh.
	    execute_process(
	      COMMAND ${TK_WISH} ${CMAKE_CURRENT_BINARY_DIR}/CheckTCL_VERSION.tcl
	      RESULT_VARIABLE TCL_RC
	      OUTPUT_VARIABLE _plplot_tcl_version
	      )

	    if(NOT TCL_RC AND _plplot_tcl_version STREQUAL PLPLOT_TCL_VERSION)
	      # Cross check that tclsh finds the same Tk version as wish.
	      execute_process(
		COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_BINARY_DIR}/CheckTK_VERSION.tcl
		RESULT_VARIABLE TK_RC
		OUTPUT_VARIABLE _plplot_tk_version
		)
	      if(NOT TK_RC AND _plplot_tk_version STREQUAL PLPLOT_TK_VERSION)
		message(STATUS "Tcl and Tk versions found by both tclsh and wish are identical")
		# Sanity checking of Tk version consistency _if_ a Tk
		# version string can be extracted from the Tk library
		# name.
		get_filename_component(library_name ${TK_LIBRARY} NAME)
		string(REGEX MATCH "[0-9]+[.]?[0-9]+" library_version ${library_name})
		if(library_version)
		  # If no decimal point in version insert as the second character.
		  if(NOT library_version MATCHES "[.]")
		    string(REGEX REPLACE "^([0-9])([0-9]+)$" "\\1.\\2" library_version ${library_version})
		  endif(NOT library_version MATCHES "[.]")
		  #message(STATUS "library_version = ${library_version}")
		  string(LENGTH ${library_version} length_library_version)
		  string(SUBSTRING ${PLPLOT_TK_VERSION} 0 ${length_library_version} truncated_version)
		  #message(STATUS "truncated_version = ${truncated_version}")
		  if(NOT truncated_version STREQUAL library_version)
		    message(STATUS "WARNING: Tk library version = ${library_version} that is extracted from the
   library name is not consistent with PLPLOT_TK_VERSION = ${PLPLOT_TK_VERSION}")
		  endif(NOT truncated_version STREQUAL library_version)
		endif(library_version)
	      elseif(TK_RC)
		message(STATUS "Looking for Tk version with tclsh - not found")
		message(STATUS "WARNING: setting ENABLE_tk to OFF")
		set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
	      else(NOT TK_RC AND _plplot_tk_version STREQUAL PLPLOT_TK_VERSION)
		message(STATUS "Tk version = ${_plplot_tk_version} found by tclsh is not consistent with
   Tk version = ${PLPLOT_TK_VERSION} found by wish"
		  )
		message(STATUS "WARNING: setting ENABLE_tk to OFF")
		set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
	      endif(NOT TK_RC AND _plplot_tk_version STREQUAL PLPLOT_TK_VERSION)
	    elseif(TCL_RC)
	      message(STATUS "Looking for Tcl version with wish - not found")
	      message(STATUS "WARNING: setting ENABLE_tk to OFF")
	      set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
	    else(NOT TCL_RC AND _plplot_tcl_version STREQUAL PLPLOT_TCL_VERSION)
	      message(STATUS "Tcl version = ${_plplot_tcl_version} found by wish is not consistent with
   Tcl version = ${PLPLOT_TCL_VERSION} found by tclsh"
		)
	      message(STATUS "WARNING: setting ENABLE_tk to OFF")
	      set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
	    endif(NOT TCL_RC AND _plplot_tcl_version STREQUAL PLPLOT_TCL_VERSION)
	  else(PLPLOT_TCL_VERSION STREQUAL PLPLOT_TK_VERSION)
	    message(STATUS "Tk version = ${PLPLOT_TK_VERSION} found by wish is not consistent with
   Tcl version = ${PLPLOT_TCL_VERSION} found by tclsh"
	      )
	    message(STATUS "WARNING: setting ENABLE_tk to OFF")
	    set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
	  endif(PLPLOT_TCL_VERSION STREQUAL PLPLOT_TK_VERSION)
	else(NOT TK_RC)
	  message(STATUS "Looking for Tk version with wish - not found")
	  message(STATUS "WARNING: setting ENABLE_tk to OFF")
	  set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
	endif(NOT TK_RC)
      else(NOT PLPLOT_TK_VERSION)
	set(TK_RC 0)
      endif(NOT PLPLOT_TK_VERSION)
    else(TK_WISH)
      message(STATUS "Looking for wish - not found")
      message(STATUS "WARNING: setting ENABLE_tk to OFF")
      set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
    endif(TK_WISH)
  elseif(ENABLE_tk AND NOT TK_FOUND)
    message(STATUS "Looking for Tk - not found")
    message(STATUS "WARNING: setting ENABLE_tk to OFF")
    set(ENABLE_tk OFF CACHE BOOL "Enable Tk binding" FORCE)
  endif(ENABLE_tk AND TK_FOUND)

  if(ENABLE_tk)
    get_filename_component(TK_LIBRARY_PATH ${TK_LIBRARY} PATH)
    list(APPEND TCL_TK_RPATH ${TK_LIBRARY_PATH})
    list(APPEND TCL_TK_ITCL_ITK_RPATH ${TK_LIBRARY_PATH})
  else(ENABLE_tk)
    message(STATUS "WARNING: Because Tk is disabled must disable Itk as well")
    set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
  endif(ENABLE_tk)

  if(ENABLE_itk)
    if(USE_INCRTCL_VERSION_4)
      set(SUGGESTED_ITK_VERSION 4)
    else(USE_INCRTCL_VERSION_4)
      set(SUGGESTED_ITK_VERSION 3)
    endif(USE_INCRTCL_VERSION_4)
    if(NOT PLPLOT_ITK_VERSION)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CheckITK_Available.tcl "puts -nonewline [package require Itk ${SUGGESTED_ITK_VERSION}]; exit")
      # Refine SUGGESTED_ITK_VERSION to exact value or fail.
      execute_process(
	COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_BINARY_DIR}/CheckITK_Available.tcl
	RESULT_VARIABLE ITK_RC
	OUTPUT_VARIABLE _plplot_itk_version
	)
      if(NOT ITK_RC)
	# Store result in cached variable so it will be remembered
        # when cmake is re-run, but use type of STRING rather than
        # INTERNAL to allow users to specify the value with a
        # -DPLPLOT_ITK_VERSION:STRING=<whatever> on the command line
        # for those rare cases (e.g., autobuild environment without X)
        # when "package require Itk" would error out due to the lack
        # of X or some tclsh issue.
	set(PLPLOT_ITK_VERSION ${_plplot_itk_version}
	  CACHE STRING "Itk version that will be used"
	  )
      endif(NOT ITK_RC)
    else(NOT PLPLOT_ITK_VERSION)
      set(ITCL_RC 0)
    endif(NOT PLPLOT_ITK_VERSION)
    if(NOT ITK_RC)
      message(STATUS "Looking for itk.h")
      if(NOT USE_INCRTCL_VERSION_4)
	# Search first for epa_build install location.
        find_path(ITK_INCLUDE_PATH itk.h HINTS "${TCL_INCLUDE_PATH}/itcl${PLPLOT_ITCL_VERSION}")
      endif(NOT USE_INCRTCL_VERSION_4)
      message(STATUS "PLPLOT_ITK_VERSION = ${PLPLOT_ITK_VERSION}")
      # N.B. Debian jessie itk3-dev package stores itk.h in /usr/include/itcl${SHORT_PLPLOT_ITCL_VERSION}
      find_path(ITK_INCLUDE_PATH itk.h HINTS "${TCL_INCLUDE_PATH}" PATH_SUFFIXES itcl${SHORT_PLPLOT_ITCL_VERSION})

      if(ITK_INCLUDE_PATH)
        message(STATUS "Looking for itk.h - found")
	message(STATUS "ITK_INCLUDE_PATH = ${ITK_INCLUDE_PATH}")

	message(STATUS "Checking for Itk header version consistency")
	string(REGEX REPLACE "^([0-9]*)\\.[0-9]*.*$" "\\1" ITK_MAJOR_VERSION  "${PLPLOT_ITK_VERSION}")
	string(REGEX REPLACE "^[0-9]*\\.([0-9]*).*$" "\\1" ITK_MINOR_VERSION  "${PLPLOT_ITK_VERSION}")
	message(STATUS "ITK_MAJOR_VERSION = ${ITK_MAJOR_VERSION}")
	message(STATUS "ITK_MINOR_VERSION = ${ITK_MINOR_VERSION}")
	if(NOT DEFINED CONSISTENT_HEADER_ITK_VERSION)
	  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/CheckITK_VERSION.c
	    "// #define RC_INVOKED to avoid sucking in lots of additional includes for
// efficiency and simplicity and also to avoid the header file disaster that has long
// been a problem for version 3.3 of itk.h
#define RC_INVOKED 1
#include <itk.h>
#if ITK_MAJOR_VERSION != ${ITK_MAJOR_VERSION} || ITK_MINOR_VERSION != ${ITK_MINOR_VERSION}
  #error ITK_VERSION mismatch
#endif
void main(void){}
"
	    )
	  try_compile(CONSISTENT_HEADER_ITK_VERSION ${PROJECT_BINARY_DIR}
	    ${CMAKE_CURRENT_BINARY_DIR}/CheckITK_VERSION.c
	    CMAKE_FLAGS "-DINCLUDE_DIRECTORIES:PATH=${ITK_INCLUDE_PATH};${TK_INCLUDE_PATH};${TCL_INCLUDE_PATH}"
	    )
	endif(NOT DEFINED CONSISTENT_HEADER_ITK_VERSION)

	if(CONSISTENT_HEADER_ITK_VERSION)
	  message(STATUS "Checking for Itk header version consistency - true")
	else(CONSISTENT_HEADER_ITK_VERSION)
	  message(STATUS "Checking for Itk header version consistency - false")
          message(STATUS "WARNING: Disabling Itk interface code")
	  set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
	endif(CONSISTENT_HEADER_ITK_VERSION)
      else(ITK_INCLUDE_PATH)
        message(STATUS "Looking for itk.h - not found")
        message(STATUS "WARNING: Disabling Itk interface code")
        set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
      endif(ITK_INCLUDE_PATH)

      if(ENABLE_itk)
        message(STATUS "Looking for itk library")
	# Search first for epa_build install location.
	string(REGEX REPLACE "\\." "" COMPACT_PLPLOT_ITK_VERSION ${PLPLOT_ITK_VERSION})
        find_library(ITK_LIBRARY NAMES itk${PLPLOT_ITK_VERSION} itk${COMPACT_PLPLOT_ITK_VERSION} itk HINTS ${TK_LIBRARY_PATH}/itk${PLPLOT_ITK_VERSION})
        find_library(ITK_LIBRARY NAMES itk${PLPLOT_ITK_VERSION} itk${COMPACT_PLPLOT_ITK_VERSION} itk HINTS ${TK_LIBRARY_PATH})
        if(ITK_LIBRARY)
          message(STATUS "Looking for itk library - found")
	  message(STATUS "ITK_LIBRARY = ${ITK_LIBRARY}")

	  # Test version consistency between iwidgets, itk, and itcl.
	  if(NOT IWIDGETS_VERSIONS_LIST)
	    if(USE_INCRTCL_VERSION_4)
	      set(SUGGESTED_IWIDGETS_VERSION 4.1)
	      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/determine_versions.tcl "puts -nonewline \"[package require Iwidgets ${SUGGESTED_IWIDGETS_VERSION}];[package require Itk];[package require Itcl]\";exit")
	      execute_process(
		COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_BINARY_DIR}/determine_versions.tcl
		RESULT_VARIABLE IWIDGETS_RC
		OUTPUT_VARIABLE _iwidgets_versions_list
		)
	    else(USE_INCRTCL_VERSION_4)
	      # Find maximum 4.0 version of iwidgets that is
	      # available.  4.0.1 (released in 2002) is the largest
	      # version of iwidgets4.0 (as opposed to the recently
	      # developed iwidgets4.1) I have found on the web in
	      # tarball form, but I have seen one reference to 4.0.2
	      # which may have been publicly released somewhere
	      # inaccessible to my google searches or privately made
	      # available before all development of iwidgets4.0
	      # stopped. So add 4.0.2 to the list just in case.
	      foreach(SUGGESTED_IWIDGETS_VERSION 4.0.2 4.0.1 4.0.0)
		file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/determine_versions.tcl "puts -nonewline \"[package require -exact Iwidgets ${SUGGESTED_IWIDGETS_VERSION}];[package require Itk];[package require Itcl]\";exit")
		execute_process(
		  COMMAND ${TCL_TCLSH} ${CMAKE_CURRENT_BINARY_DIR}/determine_versions.tcl
		  RESULT_VARIABLE IWIDGETS_RC
		  OUTPUT_VARIABLE _iwidgets_versions_list
		  ERROR_QUIET
		  )
		if(NOT IWIDGETS_RC)
		  break()
		endif(NOT IWIDGETS_RC)
	      endforeach(SUGGESTED_IWIDGETS_VERSION 4.0.2 4.0.1 4.0.0)
	    endif(USE_INCRTCL_VERSION_4)
	    if(NOT IWIDGETS_RC)
	      # Store result in cached variable so it will be
	      # remembered when cmake is re-run, but use type of
	      # STRING rather than INTERNAL to allow users to
	      # specify the value with a
	      # -DIWIDGETS_VERSIONS_LIST:STRING=<whatever> on the
	      # command line for those rare cases (e.g., autobuild
	      # environment without X) when "package require Itk (or
	      # Itcl or IWidgets)" would error out due to the lack of
	      # X or some tclsh issue
	      set(IWIDGETS_VERSIONS_LIST ${_iwidgets_versions_list}
		CACHE STRING "list of Iwidgets; Itk, and Itcl versions (in that order) that will be used"
		)
	    endif(NOT IWIDGETS_RC)
	  else(NOT IWIDGETS_VERSIONS_LIST)
	    set(IWIDGETS_RC 0)
	  endif(NOT IWIDGETS_VERSIONS_LIST)
	  if(NOT IWIDGETS_RC)
	    message(STATUS "IWIDGETS_VERSIONS_LIST = ${IWIDGETS_VERSIONS_LIST}")
	    list(GET IWIDGETS_VERSIONS_LIST 0 PLPLOT_IWIDGETS_VERSION)
	    list(GET IWIDGETS_VERSIONS_LIST 1 CONSISTENT_ITK_VERSION)
	    list(GET IWIDGETS_VERSIONS_LIST 2 CONSISTENT_ITCL_VERSION)

	    message(STATUS "PLPLOT_IWIDGETS_VERSION = ${PLPLOT_IWIDGETS_VERSION}")
	    message(STATUS "Checking that the Iwidgets, Itk, and Itcl packages are consistent")
	    if(CONSISTENT_ITCL_VERSION STREQUAL PLPLOT_ITCL_VERSION AND CONSISTENT_ITK_VERSION STREQUAL PLPLOT_ITK_VERSION)
	      message(STATUS "Checking that the Iwidgets, Itk, and Itcl packages are consistent - true")
	      get_filename_component(ITK_LIBRARY_PATH ${ITK_LIBRARY} PATH)
	      list(APPEND TCL_TK_ITCL_ITK_RPATH ${ITK_LIBRARY_PATH})
	    else(CONSISTENT_ITCL_VERSION STREQUAL PLPLOT_ITCL_VERSION AND CONSISTENT_ITK_VERSION STREQUAL PLPLOT_ITK_VERSION)
	      message(STATUS "PLPLOT_ITK_VERSION = ${PLPLOT_ITK_VERSION}")
	      message(STATUS "CONSISTENT_ITK_VERSION = ${CONSISTENT_ITK_VERSION}")
	      message(STATUS "PLPLOT_ITCL_VERSION = ${PLPLOT_ITCL_VERSION}")
	      message(STATUS "CONSISTENT_ITCL_VERSION = ${CONSISTENT_ITCL_VERSION}")
	      message(STATUS "Checking that the Iwidgets, Itk, and Itcl packages are consistent - false")
	      message(STATUS "WARNING: Consistent combination of Iwidgets, Itk, and Itcl not found so disabling Itcl and Itk")
	      set(ENABLE_itcl OFF CACHE BOOL "Enable Itcl interface code" FORCE)
	      set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
	    endif(CONSISTENT_ITCL_VERSION STREQUAL PLPLOT_ITCL_VERSION AND CONSISTENT_ITK_VERSION STREQUAL PLPLOT_ITK_VERSION)
	  else(NOT IWIDGETS_RC)
	    message(STATUS "WARNING: Iwidgets could not be found so disabling Itcl and Itk")
	    set(ENABLE_itcl OFF CACHE BOOL "Enable Itcl interface code" FORCE)
	    set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
	  endif(NOT IWIDGETS_RC)
        else(ITK_LIBRARY)
          message(STATUS "Looking for itk library - not found")
          message(STATUS "WARNING: Disabling Itk interface code")
          set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
        endif(ITK_LIBRARY)
      endif(ENABLE_itk)
    else(NOT ITK_RC)
      message(STATUS "Itk not available or not compatible with current Tcl shell")
      message(STATUS "WARNING: Disabling Itk interface code")
      set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
    endif(NOT ITK_RC)
  endif(ENABLE_itk)

  # ------------------------------------------------------------------------
  # figure out how to build tclIndex
  # ------------------------------------------------------------------------
  set(MKTCLINDEX ${PROJECT_SOURCE_DIR}/scripts/mktclIndex)
  if(ENABLE_itcl)
    set(MKTCLINDEX_ARGS "-itcl")
  else(ENABLE_itcl)
    set(MKTCLINDEX_ARGS "-tcl")
  endif(ENABLE_itcl)

  filter_rpath(TCL_RPATH)
  if(TCL_RPATH)
    message(STATUS "TCL_RPATH = ${TCL_RPATH}")
  endif(TCL_RPATH)

  filter_rpath(TCL_TK_RPATH)
  if(TCL_TK_RPATH)
    message(STATUS "TCL_TK_RPATH = ${TCL_TK_RPATH}")
  endif(TCL_TK_RPATH)

  filter_rpath(TCL_TK_ITCL_ITK_RPATH)
  if(TCL_TK_ITCL_ITK_RPATH)
    message(STATUS "TCL_TK_ITCL_ITK_RPATH = ${TCL_TK_ITCL_ITK_RPATH}")
  endif(TCL_TK_ITCL_ITK_RPATH)

  message(STATUS "Finished determining consistent system data for Tcl and friends")
else(ENABLE_tcl)
  message(STATUS "WARNING: ENABLE_tcl is OFF so disabling everything else that is Tcl/Tk related")
  set(ENABLE_itcl OFF CACHE BOOL "Enable Itcl interface code" FORCE)
  set(ENABLE_tk OFF CACHE BOOL "Enable Tk interface code" FORCE)
  set(ENABLE_itk OFF CACHE BOOL "Enable Itk interface code" FORCE)
endif(ENABLE_tcl)


if(ENABLE_tcl)
  set(
    TCL_DIR
    ${CMAKE_INSTALL_DATADIR}/${PACKAGE}${PLPLOT_VERSION}/tcl
    CACHE PATH "PLplot install location for Tcl/Tk architecture-independent data"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST TCL_DIR)
endif(ENABLE_tcl)

if(ENABLE_tk AND X11_FOUND)
  set(ENABLE_tkX ON)
else(ENABLE_tk AND X11_FOUND)
  set(ENABLE_tkX OFF)
endif(ENABLE_tk AND X11_FOUND)

if(ENABLE_itk AND X11_FOUND)
  set(ENABLE_itkX ON)
else(ENABLE_itk AND X11_FOUND)
  set(ENABLE_itkX OFF)
endif(ENABLE_itk AND X11_FOUND)

if(NOT ENABLE_itcl)
  set(PLPLOT_ITCL_VERSION 0.0.0)
endif(NOT ENABLE_itcl)

if(NOT ENABLE_itk)
  set(itk_true "#")
  set(PLPLOT_ITK_VERSION 0.0.0)
  set(PLPLOT_IWIDGETS_VERSION 0.0.0)
endif(NOT ENABLE_itk)

if(ENABLE_itcl)
  set(HAVE_ITCL ON)
  find_path(HAVE_ITCLDECLS_H itclDecls.h HINTS "${ITCL_INCLUDE_PATH}")
endif(ENABLE_itcl)

if(ENABLE_itk)
  set(HAVE_ITK ON)
endif(ENABLE_itk)


