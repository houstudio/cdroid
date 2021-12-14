# -*- mode: cmake -*-
# examples/ocaml/CMakeLists.txt

# Copyright (C) 2008-2012 Andrew Ross
# Copyright (C) 2008-2019 Alan W. Irwin
# Copyright (C) 2009-2014 Hezekiah M. Carty
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
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# N.B. This file is used for both the core build (which installs the examples
# and optionally [depending on BUILD_TEST} builds them) and the installed
# examples build.  The core build has BUILD_TEST OFF or ON at user option
# and CORE_BUILD always ON.  The installed examples build always has
# BUILD_TEST ON and CORE_BUILD OFF.

# Need these escaped versions when there are spaces in the full pathnames contained in these variables.
string(REPLACE " " "\\ " CMAKE_SOURCE_DIR_ESCAPED ${CMAKE_SOURCE_DIR})
string(REPLACE " " "\\ " CMAKE_CURRENT_SOURCE_DIR_ESCAPED ${CMAKE_CURRENT_SOURCE_DIR})
string(REPLACE " " "\\ " CMAKE_BINARY_DIR_ESCAPED ${CMAKE_BINARY_DIR})
string(REPLACE " " "\\ " CMAKE_INSTALL_LIBDIR_ESCAPED ${CMAKE_INSTALL_LIBDIR})
string(REPLACE " " "\\ " OCAML_INSTALL_DIR_ESCAPED ${OCAML_INSTALL_DIR})

set(ocaml_STRING_INDICES
  "00"
  "01"
  "02"
  "03"
  "04"
  "05"
  "06"
  "07"
  "08"
  "09"
  "10"
  "11"
  "12"
  "13"
  "14"
  "15"
  "16"
  "17"
  "18"
  "19"
  "20"
  "21"
  "22"
  "23"
  "24"
  "25"
  "26"
  "27"
  "28"
  "29"
  "30"
  "31"
  "33"
  "34"
  "plot01"
  )

set(plcairo_STRING_INDICES
  "gtk_interface"
  )

# PLplot examples
if(CORE_BUILD)
  set(ocaml_SRCS)
  foreach(STRING_INDEX ${ocaml_STRING_INDICES})
    list(APPEND ocaml_SRCS x${STRING_INDEX}.ml)
  endforeach(STRING_INDEX ${ocaml_STRING_INDICES})

  if(OCAMLOPT)
    set(OCAML_EXAMPLE_COMPILER ${OCAMLOPT})
    set(OCAML_EXAMPLE_LIBEXT cmxa)
  else(OCAMLOPT)
    set(OCAML_EXAMPLE_COMPILER ${OCAMLC})
    set(OCAML_EXAMPLE_LIBEXT cma)
  endif(OCAMLOPT)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${ocaml_SRCS}
    DESTINATION ${DATA_DIR}/examples/ocaml
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/ocaml
    RENAME Makefile
    )

  install(FILES CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/ocaml
    )

endif(CORE_BUILD)

if(BUILD_TEST)
  # Default to using ocaml if ocamlopt is not available.  Make sure the proper
  # files are cleaned up after a build, depending on the compiler used.
  if (OCAMLOPT)
    set(ocaml_compiler ${OCAMLOPT})
    set(ocaml_lib_extension cmxa)
    set(ocaml_extra_extensions cmx cmi o)
  else (OCAMLOPT)
    set(ocaml_compiler ${OCAMLC})
    set(ocaml_lib_extension cma)
    set(ocaml_extra_extensions cmo cmi)
  endif (OCAMLOPT)

  # set ccopt (important: using trailing blank) and I_OPTION.
  # build_tree_OCAML_RPATH_ESCAPED and install_tree_OCAML_RPATH_ESCAPED inherited from
  # parent directory.
  if(CORE_BUILD)
    # The unescaped I option works for both ocamlopt and ocamlc, but the escaped version
    # does not work for ocamlopt (and ocamlc?).  WTF?
    set(I_OPTION ${CMAKE_BINARY_DIR}/bindings/ocaml)
    set(ccopt_OPTION -ccopt "-L ${CMAKE_BINARY_DIR_ESCAPED}/src -Wl,-rpath -Wl,${build_tree_ocaml_RPATH_ESCAPED} ")
  else(CORE_BUILD)
    set(I_OPTION ${OCAML_INSTALL_DIR}/plplot)
    if(USE_RPATH AND install_tree_ocaml_RPATH_ESCAPED)
      set(ccopt_OPTION -ccopt "-L ${CMAKE_INSTALL_LIBDIR_ESCAPED} -Wl,-rpath -Wl,${install_tree_ocaml_RPATH_ESCAPED} ")
    else(USE_RPATH AND install_tree_ocaml_RPATH_ESCAPED)
      set(ccopt_OPTION -ccopt "-L ${CMAKE_INSTALL_LIBDIR_ESCAPED} ")
    endif(USE_RPATH AND install_tree_ocaml_RPATH_ESCAPED)
  endif(CORE_BUILD)

  get_property(files_plplot_ocaml GLOBAL PROPERTY FILES_plplot_ocaml)
  set_property(GLOBAL PROPERTY TARGETS_examples_ocaml)
  set_property(GLOBAL PROPERTY FILES_examples_ocaml)
  foreach(STRING_INDEX ${ocaml_STRING_INDICES})
    set(EXECUTABLE_NAME x${STRING_INDEX}ocaml)
    set(SOURCE_FILE x${STRING_INDEX}.ml)
    set(ocaml_EXTRA_GENERATED_FILES)
    foreach(EXTENSION ${ocaml_extra_extensions})
      list(APPEND ocaml_EXTRA_GENERATED_FILES ${CMAKE_CURRENT_BINARY_DIR}/x${STRING_INDEX}.${EXTENSION})
    endforeach(EXTENSION ocaml_extra_extensions)
    if(STRING_INDEX STREQUAL "17")
      set(unix_lib unix.${ocaml_lib_extension})
    else(STRING_INDEX STREQUAL "17")
      set(unix_lib)
    endif(STRING_INDEX STREQUAL "17")

    add_custom_command(
      OUTPUT
      ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}
      ${ocaml_EXTRA_GENERATED_FILES}
      ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
      # Need source file in build tree in order for OCaml build tools to work.
      COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
      ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}
      COMMAND ${ocaml_compiler}
      -g
      -I ${I_OPTION}
      ${ccopt_OPTION}
      plplot.${ocaml_lib_extension}
      ${unix_lib}
      -o ${EXECUTABLE_NAME}
      ${SOURCE_FILE}
      DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
      ${files_plplot_ocaml}
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
      VERBATIM
      )
    add_custom_target(
      target_${EXECUTABLE_NAME} ALL
      DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}
      ${ocaml_EXTRA_GENERATED_FILES}
      ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
      )
    if(CORE_BUILD)
      add_dependencies(target_${EXECUTABLE_NAME} plplot_ocaml)
    endif(CORE_BUILD)
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_ocaml target_${EXECUTABLE_NAME})
    set_property(GLOBAL APPEND PROPERTY FILES_examples_ocaml ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME})
  endforeach(STRING_INDEX ${ocaml_STRING_INDICES})
  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${ocaml_EXTRA_CLEAN_FILES}")
endif(BUILD_TEST)

# Plcairo examples  (N.B. OCAML_HAS_CAIRO is indefinitely disabled so the stanzas below are unmaintained.)
if(OCAML_HAS_CAIRO AND OCAML_HAS_GTK)

  if(CORE_BUILD)
    set(ocaml_SRCS)
    foreach(STRING_INDEX ${plcairo_STRING_INDICES})
      list(APPEND ocaml_SRCS x${STRING_INDEX}.ml)
    endforeach(STRING_INDEX ${plcairo_STRING_INDICES})

    install(FILES ${ocaml_SRCS}
      DESTINATION ${DATA_DIR}/examples/ocaml
      )

  endif(CORE_BUILD)

  if(BUILD_TEST)
    # Default to using ocaml if ocamlopt is not available.  Make sure the proper
    # files are cleaned up after a build, depending on the compiler used.
    if (OCAMLOPT)
      set(ocaml_compiler opt)
      set(ocaml_lib_extension cmxa)
      set(ocaml_cleanup_extensions cmx cmi o)
    else (OCAMLOPT)
      set(ocaml_compiler c)
      set(ocaml_lib_extension cma)
      set(ocaml_cleanup_extensions cmo cmi)
    endif (OCAMLOPT)

    set(ocaml_EXTRA_CLEAN_FILES)
  # set ccopt (important: using trailing blank) and I_OPTION.
  # build_tree_ocaml_RPATH and install_tree_ocaml_rpath inherited from
  # parent directory.
    if(CORE_BUILD)
      set(I_OPTION_PLPLOT ${CMAKE_BINARY_DIR}/bindings/ocaml)
      set(I_OPTION_PLCAIRO ${CMAKE_BINARY_DIR}/bindings/ocaml/plcairo)
      set(ccopt_OPTION "-L ${CMAKE_BINARY_DIR}/src -Wl,-rpath -Wl,${build_tree_ocaml_RPATH} ")
    else(CORE_BUILD)
      set(I_OPTION_PLPLOT ${OCAML_INSTALL_DIR}/plplot)
      set(I_OPTION_PLCAIRO ${OCAML_INSTALL_DIR}/plcairo)
      if(USE_RPATH AND install_tree_ocaml_RPATH)
	set(ccopt_OPTION "-L ${CMAKE_INSTALL_LIBDIR} -Wl,-rpath -Wl,${install_tree_ocaml_RPATH} ")
      else(USE_RPATH AND INSTALL_TREE_OCAML_RPATH)
	set(ccopt_OPTION "-L ${CMAKE_INSTALL_LIBDIR} ")
      endif(USE_RPATH AND install_tree_ocaml_RPATH)
    endif(CORE_BUILD)

    get_property(files_plcairo_ocaml GLOBAL PROPERTY FILES_plcairo_ocaml)
    # These are set above, in the core PLplot examples section
    #set_property(GLOBAL PROPERTY TARGETS_examples_ocaml)
    #set_property(GLOBAL PROPERTY FILES_examples_ocaml)
    foreach(STRING_INDEX ${plcairo_STRING_INDICES})
      set(EXECUTABLE_NAME x${STRING_INDEX}ocaml)
      set(SOURCE_FILE x${STRING_INDEX}.ml)
      foreach(EXTENSION ${ocaml_cleanup_extensions})
	set(ocaml_EXTRA_CLEAN_FILES ${ocaml_EXTRA_CLEAN_FILES} x${STRING_INDEX}.${EXTENSION})
      endforeach(EXTENSION ocaml_cleanup_extensions)

      # Copy source code to build tree since ocamlc is a bit broken and will
      # otherwise litter the source tree with intermediate files
      add_custom_command(
	OUTPUT
	${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
	${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE}
	)

      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
	COMMAND ${OCAMLFIND} ${ocaml_compiler}
	-package cairo2.lablgtk2
	-linkpkg
	-g
	-I ${I_OPTION_PLPLOT}
	-I ${I_OPTION_PLCAIRO}
	-ccopt ${ccopt_OPTION}
	plplot.${ocaml_lib_extension}
	plcairo.${ocaml_lib_extension}
	-o ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
	${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}
	DEPENDS
	${CMAKE_CURRENT_BINARY_DIR}/${SOURCE_FILE}
	${files_plplot_ocaml}
	${files_plcairo_ocaml}
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
	VERBATIM
	)
      add_custom_target(
	target_${EXECUTABLE_NAME} ALL
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME}
	)
      if(CORE_BUILD)
	add_dependencies(target_${EXECUTABLE_NAME} plcairo_ocaml cairo)
      endif(CORE_BUILD)
      set_property(GLOBAL APPEND PROPERTY TARGETS_examples_ocaml target_${EXECUTABLE_NAME})
      set_property(GLOBAL APPEND PROPERTY FILES_examples_ocaml ${CMAKE_CURRENT_BINARY_DIR}/${EXECUTABLE_NAME})
    endforeach(STRING_INDEX ${ocaml_STRING_INDICES})
  endif(BUILD_TEST)
endif(OCAML_HAS_CAIRO AND OCAML_HAS_GTK)
