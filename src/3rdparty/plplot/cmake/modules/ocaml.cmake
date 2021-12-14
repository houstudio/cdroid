# cmake/modules/ocaml.cmake
#
# Copyright (C) 2008 Andrew Ross
# Copyright (C) 2009 Hezekiah M. Carty
# Copyright (C) 2009-2017 Alan W. Irwin
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

# Module for determining OCaml binding configuration options

if(DEFAULT_NO_BINDINGS)
  option(ENABLE_ocaml "Enable OCaml binding" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_ocaml "Enable OCaml binding" ON)
endif(DEFAULT_NO_BINDINGS)

if(ENABLE_ocaml AND NOT PL_DOUBLE)
  message(STATUS "WARNING: "
    "Only single-precision floating point. Disabling Ocaml binding because of run-time errors in this case.")
  set(ENABLE_ocaml OFF CACHE BOOL "Enable Ocaml binding" FORCE)
endif(ENABLE_ocaml AND NOT PL_DOUBLE)

if(ENABLE_ocaml)
  find_program(OCAMLC NAMES ocamlc.opt ocamlc)
  if (OCAMLC)
    message(STATUS "OCAMLC = ${OCAMLC}")
  else (OCAMLC)
    message(STATUS "WARNING: "
      "The ocamlc application not found. Disabling OCaml binding")
    set(ENABLE_ocaml OFF CACHE BOOL "Enable OCaml binding" FORCE)
  endif (OCAMLC)
endif(ENABLE_ocaml)

if(ENABLE_ocaml)
  find_program(CAMLIDL camlidl)
  if (CAMLIDL)
    message(STATUS "CAMLIDL = ${CAMLIDL}")
  else (CAMLIDL)
    message(STATUS "WARNING: "
      "The camlidl application not found. Disabling OCaml binding")
    set(ENABLE_ocaml OFF CACHE BOOL "Enable OCaml binding" FORCE)
  endif (CAMLIDL)
endif(ENABLE_ocaml)

if(ENABLE_ocaml)
  find_program(OCAMLMKLIB ocamlmklib)
  if (OCAMLMKLIB)
    message(STATUS "OCAMLMKLIB = ${OCAMLMKLIB}")
  else (OCAMLMKLIB)
    message(STATUS "WARNING: "
      "The ocamlmklib application not found. Disabling OCaml binding")
    set(ENABLE_ocaml OFF CACHE BOOL "Enable OCaml binding" FORCE)
  endif (OCAMLMKLIB)
endif(ENABLE_ocaml)

if(ENABLE_ocaml)
  option(FORCE_OCAMLC "Force use of ocamlc rather than ocamlopt?" OFF)
  if(NOT FORCE_OCAMLC)
    find_program(OCAMLOPT NAMES ocamlopt.opt ocamlopt)
  endif(NOT FORCE_OCAMLC)

  if (OCAMLOPT)
    message(STATUS "OCAMLOPT = ${OCAMLOPT}")
  else (OCAMLOPT)
    message(STATUS "WARNING: "
      "The ocamlopt application not found. Disabling native code OCaml binding")
  endif (OCAMLOPT)
endif(ENABLE_ocaml)

if(ENABLE_ocaml)
  find_program(OCAMLFIND ocamlfind)
  if (OCAMLFIND)
    message(STATUS "OCAMLFIND = ${OCAMLFIND}")
  else (OCAMLFIND)
    message(STATUS "WARNING: "
      "The ocamlfind application not found.")
  endif (OCAMLFIND)
endif(ENABLE_ocaml)

if(ENABLE_ocaml AND BUILD_DOC)
  find_program(OCAMLDOC NAMES ocamldoc.opt ocamldoc)
  if (OCAMLDOC)
    message(STATUS "OCAMLDOC = ${OCAMLDOC}")
  else (OCAMLDOC)
    message(STATUS "WARNING: "
      "The ocamldoc application not found. Disabling OCaml API documentation generation")
  endif (OCAMLDOC)
endif(ENABLE_ocaml AND BUILD_DOC)

if(ENABLE_ocaml)
  execute_process(COMMAND ${OCAMLC} -version
    OUTPUT_VARIABLE OCAML_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  execute_process(COMMAND ${OCAMLC} -where
    OUTPUT_VARIABLE OCAML_LIB_PATH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  message(STATUS "OCAML_LIB_PATH = ${OCAML_LIB_PATH}")
  if(OCAMLFIND)
    execute_process(COMMAND ${OCAMLFIND} query camlidl
      OUTPUT_VARIABLE CAMLIDL_LIB_PATH
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    find_path(CAMLIDL_LIB_DIR libcamlidl.a PATHS ${CAMLIDL_LIB_PATH} ${OCAML_LIB_PATH} )
  else(OCAMLFIND)
    find_path(CAMLIDL_LIB_DIR libcamlidl.a PATHS ${OCAML_LIB_PATH} )
  endif(OCAMLFIND)
  if(CAMLIDL_LIB_DIR)
    message(STATUS "CAMLIDL_LIB_DIR = ${CAMLIDL_LIB_DIR}")
  else(CAMLIDL_LIB_DIR)
    message(STATUS "WARNING: "
      "The camlidl library not found. Disabling OCaml binding")
    set(ENABLE_ocaml OFF CACHE BOOL "Enable OCaml binding" FORCE)
  endif(CAMLIDL_LIB_DIR)

endif(ENABLE_ocaml)

if(ENABLE_ocaml)
  # Installation follows the Debian ocaml policy for want of a better
  # standard.
  set(
    OCAML_INSTALL_DIR
    ${CMAKE_INSTALL_PREFIX}/lib/ocaml
    CACHE PATH "PLplot install location for ocaml files"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST OCAML_INSTALL_DIR)

  if(ADD_SPECIAL_CONSISTENCY_CHECKING)
    option(GENERATE_PLPLOT_H_INC "Generate generated_plplot_h.inc" ON)
  else(ADD_SPECIAL_CONSISTENCY_CHECKING)
    # Leave this OFF by default since this target requires extra
    # dependencies such as libpcre-ocaml-dev package on Debian.
    option(GENERATE_PLPLOT_H_INC "Generate generated_plplot_h.inc" OFF)
  endif(ADD_SPECIAL_CONSISTENCY_CHECKING)

  if(GENERATE_PLPLOT_H_INC)
    find_program(OCAML ocaml)
    if (OCAML)
      message(STATUS "OCAML = ${OCAML}")
    else (OCAML)
      message(STATUS "WARNING: "
	"The ocaml application not found. Disabling generation of generated_plplot_h.inc")
      set(GENERATE_PLPLOT_H_INC OFF CACHE BOOL "Generate generated_plplot_h.inc" FORCE)
    endif (OCAML)
  endif(GENERATE_PLPLOT_H_INC)

  # Test for availability of topfind and pcre (and unix).
  if(GENERATE_PLPLOT_H_INC)
    set(text
"#use \"topfind\";;
#require \"unix\";;
#require \"pcre\";;
"
      )
    file(WRITE ${CMAKE_BINARY_DIR}/test_ocaml.ml ${text})
    # For some reason the return code is not set if an error occurs
    # with test_ocaml.ml so look for any stdout or stderr output as a sign
    # of an error condition.
    execute_process(
      COMMAND ${OCAML} test_ocaml.ml
      WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
      OUTPUT_VARIABLE output
      ERROR_VARIABLE error
      OUTPUT_STRIP_TRAILING_WHITESPACE
      ERROR_STRIP_TRAILING_WHITESPACE
      )
    if(output OR error)
      message(STATUS "WARNING: "
	"One or all of topfind, unix, or pcre components of ocaml are not installed.  Disabling generation of generated_plplot_h.inc")
      set(GENERATE_PLPLOT_H_INC OFF CACHE BOOL "Generate generated_plplot_h.inc" FORCE)
    endif(output OR error)

  endif(GENERATE_PLPLOT_H_INC)

  # Test for the availability of Cairo and Gtk+ bindings
  if(OCAMLFIND)
    if(PLD_extcairo)
      option(OCAML_HAS_CAIRO "OCaml has the cairo package" ON)
      # Disable since substantial maintenance is required before this component
      # of PLplot will configure, build, and/or run.
      message(STATUS "WARNING: PLplot OCaml Cairo support currently requires substantial development effort so is indefinitely disabled")
      set(OCAML_HAS_CAIRO OFF CACHE BOOL "OCaml has the cairo package" FORCE)

      if(OCAML_HAS_CAIRO)
	set(text_cairo "module C = Cairo")
	file(WRITE ${CMAKE_BINARY_DIR}/test_cairo.ml ${text_cairo})
	execute_process(
          COMMAND ${OCAMLFIND} c -package cairo2 -linkpkg test_cairo.ml -o test_cairo
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          RESULT_VARIABLE OCAML_HAS_CAIRO_RC
	  OUTPUT_QUIET
	  ERROR_QUIET
          )
	# non-zero return code indicates errors.
	if(OCAML_HAS_CAIRO_RC)
	  set(OCAML_HAS_CAIRO OFF CACHE BOOL "OCaml has the cairo package" FORCE)
	endif(OCAML_HAS_CAIRO_RC)
      endif(OCAML_HAS_CAIRO)

      if(OCAML_HAS_CAIRO)
        message(STATUS "Cairo OCaml library found")
	if(PKG_CONFIG_EXECUTABLE)
	  pkg_check_pkgconfig(cairo includedir libdir linkflags cflags version _CAIRO1)
	  # Transform from CMake form to pkg-config form required by ocaml build
	  # commands.
	  if(linkflags)
	    string(REGEX REPLACE ";" " " CAIRO_COMPILE_FLAGS "${cflags}")
	    pkg_config_link_flags(CAIRO_LINK_FLAGS "${linkflags}")
	    #message(STATUS "DEBUG: cairo linkflags = ${linkflags}")
	    #message(STATUS "DEBUG: CAIRO_LINK_FLAGS = ${CAIRO_LINK_FLAGS}")
	  else(linkflags)
            message(STATUS "WARNING: "
            "pkg-config could not find cairo module.  Disabling Plcairo module")
	  set(OCAML_HAS_CAIRO OFF CACHE BOOL "OCaml has the cairo package" FORCE)
	  endif(linkflags)
	else(PKG_CONFIG_EXECUTABLE)
          message(STATUS "WARNING: "
            "pkg-config not found.  Disabling Plcairo module")
	  set(OCAML_HAS_CAIRO OFF CACHE BOOL "OCaml has the cairo package" FORCE)
	endif(PKG_CONFIG_EXECUTABLE)
      endif(OCAML_HAS_CAIRO)

      if(OCAML_HAS_CAIRO)
	option(OCAML_HAS_GTK "OCaml has the cairo2.lablgtk2 package" ON)
      else(OCAML_HAS_CAIRO)
	set(OCAML_HAS_GTK OFF CACHE BOOL "OCaml has the cairo2.lablgtk2 package" FORCE)
      endif(OCAML_HAS_CAIRO)

      if(DROP_GTK_PLUS_2_BUILDS)
	set(OCAML_HAS_GTK OFF CACHE BOOL "OCaml has the cairo2.lablgtk2 package" FORCE)
      endif(DROP_GTK_PLUS_2_BUILDS)

      if(OCAML_HAS_GTK)
	set(text_gtk
	  "module G = Gtk
  module C = Cairo_gtk"
          )
	file (WRITE ${CMAKE_BINARY_DIR}/test_gtk.ml ${text_gtk})
	execute_process(
          COMMAND ${OCAMLFIND} c -package cairo2.lablgtk2 -linkpkg test_gtk.ml -o test_gtk
          WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
          RESULT_VARIABLE OCAML_HAS_GTK_RC
	  OUTPUT_QUIET
	  ERROR_QUIET
          )
	# non-zero return code indicates errors.
	if(OCAML_HAS_GTK_RC)
	  set(OCAML_HAS_GTK OFF CACHE BOOL "OCaml has the cairo2.lablgtk2 package" FORCE)
	endif(OCAML_HAS_GTK_RC)
      endif(OCAML_HAS_GTK)

      if(OCAML_HAS_GTK)
        message(STATUS "lablgtk2 OCaml library found")
      else(OCAML_HAS_GTK)
        # permanently disabled: message(STATUS "WARNING: lablgtk2 OCaml library not found.")
      endif(OCAML_HAS_GTK)
    endif (PLD_extcairo)
  else(OCAMLFIND)
    message(STATUS "WARNING: "
      "ocamlfind not available.  Disabling Plcairo module and lablgtk2 support")
    set(OCAML_HAS_CAIRO OFF CACHE BOOL "OCaml has the cairo package" FORCE)
    set(OCAML_HAS_GTK OFF CACHE BOOL "OCaml has the cairo2.lablgtk2 package" FORCE)
  endif(OCAMLFIND)
else(ENABLE_ocaml)
  message(STATUS "WARNING: "
    "ENABLE_ocaml is OFF so disabling Plcairo module and lablgtk2 support")
  set(OCAML_HAS_CAIRO OFF CACHE BOOL "OCaml has the cairo package" FORCE)
  set(OCAML_HAS_GTK OFF CACHE BOOL "OCaml has the cairo2.lablgtk2 package" FORCE)
endif(ENABLE_ocaml)
