# examples/python/CMakeLists.txt

# Copyright (C) 2006-2019 Alan W. Irwin
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

# N.B. examples 14, 17, and 31 handled independently while 32 has
# not yet been implemented.
set(python_STRING_INDICES
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
  "15"
  "16"
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
  "33"
  )

set(python_SCRIPTS)
set(python_DATA)
foreach(STRING_INDEX ${python_STRING_INDICES})
  set(python_SCRIPTS ${python_SCRIPTS} x${STRING_INDEX})
  set(python_DATA ${python_DATA} x${STRING_INDEX}.py)
endforeach(STRING_INDEX ${python_STRING_INDICES})

# This list contains standalone scripts which are executable and which
# should be installed in the examples/python directory.
set(
  python_SCRIPTS
  ${python_SCRIPTS}
  pythondemos.py
  x14
  x17
  x31
  x14.py
  x17.py
  x31.py
  plplot_logo.py
  test_circle.py
  test_axis_precision.py
  test_symbol_clip.py
  testh.py
  test_superscript_subscript.py
  test_plplot_encodings.py
  test_fill.py
  test_gradient.py
  test_type1.py
  test_hebrew_diacritic.py
  test_style.py
  test_linebreak.py
  )

# AND (ENABLE_DYNDRIVERS OR PLD_tk) Boolean logic required to
# insure correct code to support the _Pltk_init module is in
# libplplot if ENABLE_DYNDRIVERS is OFF.

if(ENABLE_tkX AND (ENABLE_DYNDRIVERS OR PLD_tk))
  list(APPEND python_SCRIPTS pytkdemo)
endif(ENABLE_tkX AND (ENABLE_DYNDRIVERS OR PLD_tk))

if(ENABLE_pyqt4)
  set(
    python_SCRIPTS
    ${python_SCRIPTS}
    pyqt4_example.py
    )
endif(ENABLE_pyqt4)

if(ENABLE_pyqt5)
  set(
    python_SCRIPTS
    ${python_SCRIPTS}
    pyqt5_example.py
    )
endif(ENABLE_pyqt5)

# This list contains python modules which are not executable on their own and
# data files that should be installed in the examples/python directory.

set(
  python_DATA
  ${python_DATA}
  restore.py
  README.pythondemos
  README.rendering_tests
  README.logo
  )

if(CORE_BUILD)

  if(BUILD_TEST)
    # equivalent to install commands but at "make" time rather than
    # "make install" time, to the build tree if different than the source
    # tree, and skipping plplot_python_start.py since it is already
    # in the build tree.
    set(command_depends)
    foreach(file ${python_SCRIPTS} ${python_DATA})
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
	)
      list(APPEND command_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${file})
    endforeach(file ${python_SCRIPTS} ${python_DATA})
    add_custom_target(python_examples ALL DEPENDS ${command_DEPENDS})
    set_property(GLOBAL PROPERTY TARGETS_examples_python python_examples
      PLPLOT::plplotc)
    if(ENABLE_tkX AND (ENABLE_DYNDRIVERS OR PLD_tk))
      set_property(GLOBAL APPEND PROPERTY TARGETS_examples_python python_examples
      PLPLOT::Pltk_init)
    endif(ENABLE_tkX AND (ENABLE_DYNDRIVERS OR PLD_tk))
    set_property(GLOBAL PROPERTY FILES_examples_python ${command_DEPENDS})
  endif(BUILD_TEST)

  set(PLSMEM_DEVICE_LIST)
  foreach(memdriver mem memcairo memqt)
    if(PLD_${memdriver})
      list(APPEND PLSMEM_DEVICE_LIST ${memdriver})
    endif(PLD_${memdriver})
  endforeach(memdriver mem memcairo memqt)

  if(PLSMEM_DEVICE_LIST)
    string(REGEX REPLACE ";" "\", \"" PLSMEM_DEVICE_LIST "${PLSMEM_DEVICE_LIST}")
    set(PLSMEM_DEVICE_LIST "[\"${PLSMEM_DEVICE_LIST}\"]")
  else(PLSMEM_DEVICE_LIST)
    set(PLSMEM_DEVICE_LIST "[]")
  endif(PLSMEM_DEVICE_LIST)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_plsmem.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_plsmem.py
    )

  set(PERM_SCRIPTS
    OWNER_READ
    OWNER_WRITE
    OWNER_EXECUTE
    GROUP_READ
    GROUP_EXECUTE
    WORLD_READ
    WORLD_EXECUTE
    )

  install(FILES ${python_SCRIPTS} ${CMAKE_CURRENT_BINARY_DIR}/test_plsmem.py
    DESTINATION ${DATA_DIR}/examples/python
    PERMISSIONS ${PERM_SCRIPTS}
    )

  set(PERM_DATA
    OWNER_READ
    OWNER_WRITE
    GROUP_READ
    WORLD_READ
    )
  install(FILES ${python_DATA}
    DESTINATION ${DATA_DIR}/examples/python
    PERMISSIONS ${PERM_DATA}
    )

  set(python_location1 ${CMAKE_SOURCE_DIR}/bindings/python)
  set(python_location2 ${CMAKE_BINARY_DIR}/bindings/python)
  set(python_location3 ${CMAKE_BINARY_DIR}/examples/python)
  if(USE_DLL_SUBDIRECTORY)
    set(python_location4 ${CMAKE_BINARY_DIR}/dll)
  else(USE_DLL_SUBDIRECTORY)
    set(python_location4)
  endif(USE_DLL_SUBDIRECTORY)

  if(ENABLE_pyqt4)
    set(python_location5 ${CMAKE_BINARY_DIR}/bindings/qt_gui/pyqt4)
  elseif(ENABLE_pyqt5)
    set(python_location5 ${CMAKE_BINARY_DIR}/bindings/qt_gui/pyqt5)
  else(ENABLE_pyqt4)
    set(python_location5)
  endif(ENABLE_pyqt4)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plplot_python_start.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_python_start.py
    @ONLY
    )

  set(python_location1 ${PYTHON_INSTDIR})
  set(python_location2)
  set(python_location3)
  set(python_location4)
  set(python_location5)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plplot_python_start.py.in
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_python_start.py_install
    @ONLY
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plplot_python_start.py_install
    DESTINATION ${DATA_DIR}/examples/python
    PERMISSIONS ${PERM_DATA}
    RENAME plplot_python_start.py
    )

  install(FILES CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/python
    )

else(CORE_BUILD)
  set_property(GLOBAL PROPERTY FILES_examples_python)
  foreach(file ${python_SCRIPTS} ${python_DATA} plplot_python_start.py test_plsmem.py)
    set_property(GLOBAL APPEND PROPERTY FILES_examples_python
      ${CMAKE_CURRENT_SOURCE_DIR}/${file}
      )
  endforeach(file ${python_SCRIPTS} ${python_DATA} plplot_python_start.py test_plsmem.py)
endif(CORE_BUILD)

# Custom target to build logo (Unix only with ImageMagick installed).
if(BUILD_TEST)
  if(CORE_BUILD)
    set(working_directory ${CMAKE_CURRENT_BINARY_DIR})
  else(CORE_BUILD)
    set(working_directory ${CMAKE_CURRENT_SOURCE_DIR})
  endif(CORE_BUILD)
  set(command_file ${working_directory}/plplot_logo.py)
  add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_logo.svg
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_logo.jpg
    COMMAND ${command_file} -dev svg -geometry 760x120
    -o ${CMAKE_CURRENT_BINARY_DIR}/plplot_logo.svg
    COMMAND convert
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_logo.svg
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_logo.jpg
    DEPENDS ${command_file}
    WORKING_DIRECTORY ${working_directory}
    )
  add_custom_target(plplot_logo
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_logo.svg
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_logo.jpg
    )
  if(CORE_BUILD)
    add_dependencies(plplot_logo python_examples PLPLOT::plplotc)
  else(CORE_BUILD)
    add_dependencies(plplot_logo PLPLOT::plplotc)
  endif(CORE_BUILD)

endif(BUILD_TEST)
