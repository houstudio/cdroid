# examples/tk/CMakeLists.txt
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
# along with PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# N.B. This file is used for both the core build (which installs the examples
# and optionally [depending on BUILD_TEST} builds them) and the installed
# examples build.  The core build has BUILD_TEST OFF or ON at user option
# and CORE_BUILD always ON.  The installed examples build always has
# BUILD_TEST ON and CORE_BUILD OFF.

set(tk_STRING_INDICES
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
  )

set(tk_SRC_FILES
  README.tkdemos
  runAllDemos.tcl
  runExtendedDemos.tcl
  tkdemos.tcl
  )

# Copy files to the binary directory (if different) for generating tclIndex
# This ensures no files are created in the source tree.
set(tk_FILES)
foreach(_file ${tk_SRC_FILES})
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}
    COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
    ${CMAKE_CURRENT_BINARY_DIR}/${_file}
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${_file}
    )
  set(tk_FILES ${tk_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${_file})
endforeach(_file ${tk_SRC_FILES})

if(CORE_BUILD)
  set(tcl_EXAMPLES_PREFIX ${PROJECT_SOURCE_DIR}/examples/tcl)
else(CORE_BUILD)
  set(tcl_EXAMPLES_PREFIX ${PROJECT_SOURCE_DIR}/tcl)
endif(CORE_BUILD)

foreach(STRING_INDEX ${tk_STRING_INDICES})
  set(_file x${STRING_INDEX}.tcl)
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${_file}
    COMMAND ${CMAKE_COMMAND} -E copy
    ${tcl_EXAMPLES_PREFIX}/${_file}
    ${CMAKE_CURRENT_BINARY_DIR}/${_file}
    DEPENDS ${tcl_EXAMPLES_PREFIX}/${_file}
    )
  set(tk_FILES ${tk_FILES} ${CMAKE_CURRENT_BINARY_DIR}/${_file})
endforeach(STRING_INDEX ${tk_STRING_INDICES})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/plgrid.tcl
  COMMAND ${CMAKE_COMMAND} -E copy
  ${tcl_EXAMPLES_PREFIX}/plgrid.tcl
  ${CMAKE_CURRENT_BINARY_DIR}/plgrid.tcl
  DEPENDS ${tcl_EXAMPLES_PREFIX}/plgrid.tcl
  )
set(tk_FILES ${tk_FILES} ${CMAKE_CURRENT_BINARY_DIR}/plgrid.tcl)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
  COMMAND ${TCL_TCLSH} ${MKTCLINDEX} ${MKTCLINDEX_ARGS}
  DEPENDS ${tk_FILES}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )

add_custom_target(tclIndex_examples_tk ALL
  DEPENDS ${tk_FILES} ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
  )

set(tk_SRC)
if(ENABLE_tkX)
  list(APPEND tk_SRC xtk01.c)
endif(ENABLE_tkX)

# The third example is run using a configured shell script which invokes
# plserver so there is no source code to be compiled.

# The second and fourth Tk examples depend on Itk to work.
if(ENABLE_itkX)
  list(APPEND tk_SRC xtk02.c xtk04.c)
endif(ENABLE_itkX)

# The first, second, and fourth tk examples depend on configured bang
# scripts to work which depends directly on the executables, xtk01,
# xtk02, and xtk04.

# The third example is run with the configured shell script, tk03, which
# invokes plserver.

# plgrid is a configured shell script that runs the plgrid.tcl example
# (N.B., in the tcl directory) using plserver.

# plserver_standard_examples is a configured shell script that runs
# all the standard examples under plserver.

# N.B. "gen_" prefix means the configured script has a generator expression.
# while "non_gen_" prefix means the configured script does not have a generator expression.
set(gen_tk_SCRIPTS
  tk01
  tk03
  plgrid
  plserver_standard_examples
  plserver_runAllDemos
  )

set(non_gen_tk_SCRIPTS
  wish_standard_examples
  wish_runAllDemos
)

# These examples depend on Itk to work.
if(ENABLE_itkX)
  list(APPEND gen_tk_SCRIPTS
    tk02
    tk04
    )
endif(ENABLE_itkX)

set(tk_SCRIPTS ${gen_tk_SCRIPTS} ${non_gen_tk_SCRIPTS})

if(BUILD_TEST)
  if(CORE_BUILD)
    include_directories(
      ${TCL_INCLUDE_PATH}
      ${TK_INCLUDE_PATH}
      ${PROJECT_SOURCE_DIR}/bindings/tcl
      ${PROJECT_SOURCE_DIR}/bindings/tk
      ${PROJECT_SOURCE_DIR}/include
      ${PROJECT_BINARY_DIR}/include
      )
  else(CORE_BUILD)
    include_directories(
      ${TCL_INCLUDE_PATH}
      ${TK_INCLUDE_PATH}
      ${INCLUDE_DIR}
      )
  endif(CORE_BUILD)

  set_property(GLOBAL PROPERTY TARGETS_examples_tk)
  foreach(TK_SRC_FILE ${tk_SRC})
    string(REGEX REPLACE ".c$" "" TK_EXE ${TK_SRC_FILE})
    add_executable(${TK_EXE} ${TK_SRC_FILE})
    if(BUILD_SHARED_LIBS)
      set_target_properties(${TK_EXE} PROPERTIES
	COMPILE_DEFINITIONS "USINGDLL"
	)
    endif(BUILD_SHARED_LIBS)
    if(tcltk_in_plplot_library)
      target_link_libraries(${TK_EXE} PLPLOT::plplottcltk_Main PLPLOT::plplot ${TK_LIBRARY} ${TCL_LIBRARY} ${MATH_LIB})
    else(tcltk_in_plplot_library)
      target_link_libraries(${TK_EXE} PLPLOT::plplottcltk_Main PLPLOT::plplottcltk PLPLOT::tclmatrix PLPLOT::plplot ${TK_LIBRARY} ${TCL_LIBRARY} ${MATH_LIB})
    endif(tcltk_in_plplot_library)
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_tk ${TK_EXE})
  endforeach(TK_SRC_FILE ${tk_SRC})

  set_property(GLOBAL APPEND PROPERTY TARGETS_examples_tk tclIndex_examples_tk)

  # Provide locations for several tk-related targets that will be used
  # to configure the tk_SCRIPTS.

  # wish_standard_examples is a configured shell script that runs all
  # the standard examples under wish using a TEA-based approach.
  # pkgIndex_LOCATION is the directory where the relevant pkgIndex.tcl
  # file is located.  In the Plplotter package case two build-tree
  # locations must be given, and we use the pkgIndex_LOCATIONS variable
  # (appropriately quoted for the space in source, build, or install tree prefix
  # case) to configure that case.
  if(CORE_BUILD)
    set(pkgIndex_LOCATION ${PROJECT_BINARY_DIR}/bindings/tk)
    set(pkgIndex_LOCATIONS "\"${PROJECT_BINARY_DIR}/bindings/tk\" \"${PROJECT_BINARY_DIR}/bindings/tk-x-plat\"")
  else(CORE_BUILD)
    set(pkgIndex_LOCATION ${DATA_DIR})
    set(pkgIndex_LOCATIONS "\"${DATA_DIR}\"")
  endif(CORE_BUILD)

  # N.B. the prefixed versus unprefixed logic is to avoid the invalid
  #  character ":" in the ${tk_related_target}_LOCATION variable name!
  foreach(prefixed_tk_related_target xtk01 PLPLOT::plserver xtk02 xtk04)
    string(REGEX REPLACE "PLPLOT::" "" tk_related_target ${prefixed_tk_related_target})
    if(TARGET ${prefixed_tk_related_target})
      set(${tk_related_target}_LOCATION $<TARGET_FILE:${prefixed_tk_related_target}>)
      #message(STATUS "DEBUG: ${tk_related_target}_LOCATION = ${${tk_related_target}_LOCATION}")
    endif(TARGET ${prefixed_tk_related_target})
  endforeach(prefixed_tk_related_target xtk01 PLPLOT::plserver xtk02 xtk04)

  # Configure the tk_SCRIPTS
  foreach(tk_SCRIPT ${non_gen_tk_SCRIPTS})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${tk_SCRIPT}.in
      ${CMAKE_CURRENT_BINARY_DIR}/${tk_SCRIPT}
      @ONLY
      )
  endforeach(tk_SCRIPT ${non_gen_tk_SCRIPTS})

  foreach(tk_SCRIPT ${gen_tk_SCRIPTS})
    # N.B. Results only available at generate time.
    configure_file_generate(
      ${CMAKE_CURRENT_SOURCE_DIR}/${tk_SCRIPT}.in
      ${CMAKE_CURRENT_BINARY_DIR}/${tk_SCRIPT}
      @ONLY
      )
  endforeach(tk_SCRIPT ${gen_tk_SCRIPTS})
endif(BUILD_TEST)

if(CORE_BUILD)
  # Provide install locations for several tk-related targets that will
  # be used to configure the tk_SCRIPTS used in the traditional
  # Makefile+pkg-config build system for the installed examples.
  foreach(tk_related_target xtk01 plserver xtk02 xtk04)
    if(tk_related_target STREQUAL "plserver")
      set(install_dir ${BIN_DIR})
    else(tk_related_target STREQUAL "plserver")
      set(install_dir ${DATA_DIR}/examples/tk)
    endif(tk_related_target STREQUAL "plserver")
    set(${tk_related_target}_LOCATION
      ${install_dir}/${tk_related_target}${EXEEXT}
      )
  endforeach(tk_related_target xtk01 plserver xtk02 xtk04)

  foreach(tk_SCRIPT ${tk_SCRIPTS})
    # Configure and install the tk_SCRIPTS used for the traditional build.
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${tk_SCRIPT}.in
      ${CMAKE_CURRENT_BINARY_DIR}/install_${tk_SCRIPT}
      @ONLY
      )
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/install_${tk_SCRIPT}
      DESTINATION ${DATA_DIR}/examples/tk
      RENAME ${tk_SCRIPT}
      )
    # install the template files used to configure tk_SCRIPTS for the
    # new installed examples build system.
    install(PROGRAMS ${CMAKE_CURRENT_SOURCE_DIR}/${tk_SCRIPT}.in
      DESTINATION ${DATA_DIR}/examples/tk
      )
  endforeach(tk_SCRIPT ${tk_SCRIPTS})
endif(CORE_BUILD)

if(CORE_BUILD)
  install(FILES ${tk_FILES} DESTINATION ${DATA_DIR}/examples/tk)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tclIndex DESTINATION ${DATA_DIR}/examples/tk)

  install(FILES ${tk_SRC} DESTINATION ${DATA_DIR}/examples/tk)

  set(CC "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1} ${CMAKE_C_FLAGS}")

  # Transform TK_LIBRARY and TCL_LIBRARY from list of libraries to the standard pkg-config form
  # to help configure Makefile.examples below.
  pkg_config_link_flags(pkg_config_TK_LIBRARY "${TK_LIBRARY}")
  pkg_config_link_flags(pkg_config_TCL_LIBRARY "${TCL_LIBRARY}")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/tk
    RENAME Makefile
    )

  install(FILES CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/tk
    )
endif(CORE_BUILD)
