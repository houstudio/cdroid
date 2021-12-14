# bindings/CMakeLists.txt for PLplot
###
### Process this file with cmake to produce Makefile
###
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

# swig-support has to come before swig-generated bindings (currently
# python, java, lua, and octave).
add_subdirectory(swig-support)
add_subdirectory(c++)
add_subdirectory(fortran)
add_subdirectory(tcl)
add_subdirectory(tk)
add_subdirectory(tk-x-plat)
add_subdirectory(python)
add_subdirectory(octave)
add_subdirectory(java)
add_subdirectory(wxwidgets)
add_subdirectory(ada)
add_subdirectory(d)
add_subdirectory(ocaml)
add_subdirectory(lua)
add_subdirectory(qt_gui)

set(index)
if(ENABLE_tcl)
  list(APPEND index "${CMAKE_CURRENT_BINARY_DIR}/tcl/pkgIndex.tcl")
endif(ENABLE_tcl)
if(PLD_tk)
  list(APPEND index "${CMAKE_CURRENT_BINARY_DIR}/tk/pkgIndex.tcl")
endif(PLD_tk)
if(PLD_tkwin)
  list(APPEND index "${CMAKE_CURRENT_BINARY_DIR}/tk-x-plat/pkgIndex.tcl")
endif(PLD_tkwin)

if(index)
  # The best cross-platform method of concatenating these files would
  # be to use "cmake -E concatenate ...", but that functionality has
  # not been implemented yet so we use a different method.
  # This method of concatenating files at cmake time works because each
  # of the files in index is configured at cmake time.
  # N.B. the quotes on filecontents below are necessary to avoid
  # having semicolons in filecontents messing up the results.
  file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/concatenate_pkgIndex.tcl.cmake
"
set(script_index \"${index}\")
#message(STATUS \"script_index = \${script_index}\")
file(WRITE \"${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl\" \"\")
foreach(filename IN LISTS script_index)
  #message(STATUS \"filename = \${filename}\")
  file(READ \${filename} filecontents)
  file(APPEND \"${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl\" \"\${filecontents}\")
endforeach(filename IN LISTS script_index)
"
  )

  add_custom_command(
    OUTPUT
    ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
    COMMAND
    ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/concatenate_pkgIndex.tcl.cmake
    DEPENDS
    ${index}
    VERBATIM
    )
  add_custom_target(
    concatenate_pkgIndex.tcl ALL
    DEPENDS
    ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
    )

  install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
  DESTINATION ${DATA_DIR}
  )
endif(index)
