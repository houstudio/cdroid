# examples/lua/CMakeLists.txt
# Copyright (C) 2009-2018 Alan W. Irwin
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

set(lua_STRING_INDICES
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

set(lua_SCRIPTS)
foreach(STRING_INDEX ${lua_STRING_INDICES})
  set(lua_SCRIPTS ${lua_SCRIPTS} x${STRING_INDEX}.lua)
endforeach(STRING_INDEX ${lua_STRING_INDICES})

if(CORE_BUILD)
  # These permissions because lua scripts are not standalone.
  set(PERM_DATA
    OWNER_READ
    OWNER_WRITE
    GROUP_READ
    WORLD_READ
    )

  install(FILES ${lua_SCRIPTS}
    DESTINATION ${DATA_DIR}/examples/lua
    PERMISSIONS ${PERM_DATA}
    )

  if(BUILD_TEST)
    # equivalent to install commands but at "make" time rather than
    # "make install" time, to the build tree if different than the source
    # tree.
    set(command_depends)
    foreach(file ${lua_SCRIPTS})
      add_custom_command(
	OUTPUT
	${CMAKE_CURRENT_BINARY_DIR}/${file}
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_CURRENT_SOURCE_DIR}/${file}
	${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/${file}
	)
      list(APPEND command_DEPENDS
	${CMAKE_CURRENT_BINARY_DIR}/${file}
	)
    endforeach(file ${lua_SCRIPTS})
    add_custom_target(lua_examples ALL DEPENDS ${command_DEPENDS})
    set_property(GLOBAL PROPERTY TARGETS_examples_lua lua_examples PLPLOT::plplotluac)
    set_property(GLOBAL PROPERTY FILES_examples_lua ${command_DEPENDS})
  endif(BUILD_TEST)

  if(USE_DLL_SUBDIRECTORY)
    set(lua_lib_location ${CMAKE_BINARY_DIR}/dll)
  else(USE_DLL_SUBDIRECTORY)
    set(lua_lib_location ${CMAKE_BINARY_DIR}/bindings/lua)
  endif(USE_DLL_SUBDIRECTORY)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plplot_examples.lua.in
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_examples.lua
    @ONLY
    )

  set(lua_lib_location ${LUA_DIR})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plplot_examples.lua.in
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_examples.lua_install
    @ONLY
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plplot_examples.lua_install
    DESTINATION ${DATA_DIR}/examples/lua
    PERMISSIONS ${PERM_DATA}
    RENAME plplot_examples.lua
    )

  install(FILES CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/lua
    )
else(CORE_BUILD)
  set_property(GLOBAL PROPERTY TARGETS_examples_lua PLPLOT::plplotluac)
  set_property(GLOBAL PROPERTY FILES_examples_lua)
  foreach(file ${lua_SCRIPTS})
    set_property(GLOBAL APPEND PROPERTY
      FILES_examples_lua ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      )
  endforeach(file ${lua_SCRIPTS})
endif(CORE_BUILD)
