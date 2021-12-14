# utils/CMakeLists.txt for PLplot
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

include_directories(
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/lib/qsastime
  ${CMAKE_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/include
  ${CMAKE_BINARY_DIR}/lib/qsastime
  )

# plrender, pltek, both depend on the plplot library while wxPLViewer depends on
# the plplotwxwidgets and plplotcxx libraries.
set(LIB_INSTALL_RPATH ${LIB_DIR})

if(PLD_plmeta)
  configure_executable_build(plrender "plrender.c" "PLPLOT::plplot" "${LIB_INSTALL_RPATH}" ${BIN_DIR})
endif(PLD_plmeta)

configure_executable_build(pltek "pltek.c" "PLPLOT::plplot" "${LIB_INSTALL_RPATH}" ${BIN_DIR})

# Build simple executable to check parity bits of all characters read
# by stdin.
add_executable(parity_bit_check parity_bit_check.c)

if(ENABLE_wxwidgets AND NOT OLD_WXWIDGETS)
# Build wxwidgets applications with same wxwidgets compile and link flags
# as used with the PLplot wxwidgets device driver.

  set(wxPLViewer_SRCS wxplviewer.cpp wxplframe.cpp ${CMAKE_SOURCE_DIR}/drivers/wxwidgets_comms.cpp)
  set_source_files_properties(${wxPLViewer_SRCS}
    PROPERTIES COMPILE_FLAGS "${wxwidgets_COMPILE_FLAGS}")
  include_directories(
    ${CMAKE_SOURCE_DIR}/bindings/wxwidgets
    ${CMAKE_BINARY_DIR}/bindings/wxwidgets
    ${CMAKE_SOURCE_DIR}/bindings/c++
    )

  set(wxPLViewer_link_libraries PLPLOT::plplotwxwidgets PLPLOT::plplotcxx ${wxwidgets_LINK_FLAGS} ${MATH_LIB} ${RT_LIB})
  configure_executable_build(wxPLViewer "${wxPLViewer_SRCS}" "${wxPLViewer_link_libraries}" "${LIB_INSTALL_RPATH}" ${BIN_DIR})

  # Set this property rather than using the WIN32 signature of add_executable
  # inside the configure_executable_build function.
  set_target_properties(${WRITEABLE_TARGET}wxPLViewer
    PROPERTIES
    WIN32_EXECUTABLE "ON"
    )
endif(ENABLE_wxwidgets AND NOT OLD_WXWIDGETS)
