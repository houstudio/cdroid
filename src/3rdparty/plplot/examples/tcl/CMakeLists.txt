# examples/tcl/CMakeLists.txt
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

# List of unconfigured Tcl files
set(tcl_FILES
README.tcldemos
plgrid.tcl
plot.dat
plot.tcl
r.dat
stats.log
tcldemos.tcl
)

set(tcl_STRING_INDICES
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

# List of configured Tcl files without the .in suffix.
set(configured_tcl_SCRIPTS pltcl_standard_examples tclsh_standard_examples)
foreach(STRING_INDEX ${tcl_STRING_INDICES})
  # List of unconfigured Tcl files
  list(APPEND tcl_FILES x${STRING_INDEX}.tcl)
  # List of configured Tcl files without the .in suffix.
  list(APPEND configured_tcl_SCRIPTS x${STRING_INDEX})
endforeach(STRING_INDEX ${tcl_STRING_INDICES})

# pltcl_LOCATION is the generate-time location of the pltcl executable.
set(pltcl_LOCATION $<TARGET_FILE:PLPLOT::pltcl>)
# pkgIndex_LOCATION is the directory where the relevant pkgIndex.tcl
# file is located.
if(CORE_BUILD)
    set(pkgIndex_LOCATION ${CMAKE_BINARY_DIR}/bindings/tcl)
else(CORE_BUILD)
    set(pkgIndex_LOCATION ${DATA_DIR})
endif(CORE_BUILD)

# List of configured Tcl template files (i.e., with the .in suffix).
set(configured_tcl_template_SCRIPTS)
foreach(configured_tcl_SCRIPT ${configured_tcl_SCRIPTS})
  set(configured_tcl_template_SCRIPT ${configured_tcl_SCRIPT}.in)
  list(APPEND configured_tcl_template_SCRIPTS ${configured_tcl_template_SCRIPT})
  configure_file_generate(
    ${CMAKE_CURRENT_SOURCE_DIR}/${configured_tcl_template_SCRIPT}
    ${CMAKE_CURRENT_BINARY_DIR}/${configured_tcl_SCRIPT}
    @ONLY
    )
endforeach(configured_tcl_SCRIPT ${configured_tcl_SCRIPTS})

# Copy ${tcl_FILES} to the binary directory if different to the
# source directory. Needed for all CMake-based testing and also
# so that the tclIndex file is generated in the binary tree and not the source tree.
if(NOT CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")
  foreach(file ${tcl_FILES})
    add_custom_command(
      OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}
      COMMAND ${CMAKE_COMMAND} -E copy
      ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      ${CMAKE_CURRENT_BINARY_DIR}
      DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      )
  endforeach(file ${tcl_FILES})
endif(NOT CMAKE_BINARY_DIR STREQUAL "${CMAKE_SOURCE_DIR}")

set(tclIndex_DEPENDS)
foreach(file ${configured_tcl_SCRIPTS} ${tcl_FILES})
  list(APPEND tclIndex_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${file})
endforeach(file ${configured_tcl_SCRIPTS} ${tcl_FILES})
add_custom_target(tcl_examples ALL DEPENDS ${tclIndex_DEPENDS})

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
  COMMAND ${TCL_TCLSH} ${MKTCLINDEX} ${MKTCLINDEX_ARGS}
  DEPENDS ${tclIndex_DEPENDS}
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
  )
add_custom_target(tclIndex_examples_tcl ALL
  DEPENDS ${tclIndex_DEPENDS} ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
  )

# These two custom targets file-depend on common files so must serialize.
add_dependencies(tclIndex_examples_tcl tcl_examples)

if(CORE_BUILD)
  install(FILES CMakeLists.txt DESTINATION ${DATA_DIR}/examples/tcl)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/tclIndex DESTINATION ${DATA_DIR}/examples/tcl)
  install(FILES ${tcl_FILES} DESTINATION ${DATA_DIR}/examples/tcl)
  install(PROGRAMS ${configured_tcl_template_SCRIPTS} DESTINATION ${DATA_DIR}/examples/tcl)
  # Install pre-configured version of configured files for use of the traditional
  # build of the installed examples.
  set(pltcl_LOCATION ${BIN_DIR}/pltcl)
  foreach(configured_tcl_SCRIPT ${configured_tcl_SCRIPTS})
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/${configured_tcl_SCRIPT}.in
      ${CMAKE_CURRENT_BINARY_DIR}/${configured_tcl_SCRIPT}_install
      @ONLY
      )
    install(PROGRAMS ${CMAKE_CURRENT_BINARY_DIR}/${configured_tcl_SCRIPT}_install
      DESTINATION ${DATA_DIR}/examples/tcl
      RENAME ${configured_tcl_SCRIPT})
  endforeach(configured_tcl_SCRIPT ${configured_tcl_SCRIPTS})
endif(CORE_BUILD)

if(BUILD_TEST)
  if(ENABLE_DYNDRIVERS)
    set(plplottcltk_target PLPLOT::plplottcltk)
  else(ENABLE_DYNDRIVERS)
    set(plplottcltk_target PLPLOT::plplot)
  endif(ENABLE_DYNDRIVERS)

  if(CORE_BUILD)
    set_property(GLOBAL PROPERTY TARGETS_examples_tcl
      tcl_examples
      tclIndex_examples_tcl
      ${plplottcltk_target}
      PLPLOT::pltcl
      plplot.tcl_target
      tclIndex_tcl
      )
    get_property(files_plplot.tcl_target GLOBAL PROPERTY FILES_plplot.tcl_target)
    get_property(files_tclIndex_tcl GLOBAL PROPERTY FILES_tclIndex_tcl)
    set_property(GLOBAL PROPERTY FILES_examples_tcl
      ${tclIndex_DEPENDS}
      ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
      ${plplottcltk_target}
      PLPLOT::pltcl
      ${files_plplot.tcl_target}
      ${files_tclIndex_tcl}
      )
  else(CORE_BUILD)
    set_property(GLOBAL PROPERTY TARGETS_examples_tcl
      tcl_examples
      ${plplottcltk_target}
      )
    set_property(GLOBAL PROPERTY FILES_examples_tcl
      ${tclIndex_DEPENDS}
      ${plplottcltk_target}
      )
  endif(CORE_BUILD)
endif(BUILD_TEST)
