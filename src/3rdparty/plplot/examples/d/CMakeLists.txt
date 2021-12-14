# examples/d/CMakeLists.txt
# Copyright (C) 2008 Werner Smekal
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

set(d_STRING_INDICES
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

if(CORE_BUILD)
  set(d_SRCS)
  foreach(STRING_INDEX ${d_STRING_INDICES})
    set(d_SRCS ${d_SRCS} x${STRING_INDEX}d.d)
  endforeach(STRING_INDEX ${d_STRING_INDICES})

  install(FILES ${d_SRCS}
    DESTINATION ${DATA_DIR}/examples/d
    )

  set(DC "${CMAKE_D_COMPILER} ${CMAKE_D_COMPILER_ARG1} ${CMAKE_D_FLAGS}")

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/d
    RENAME Makefile
    )

  install(FILES CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/d
    )
endif(CORE_BUILD)

if(BUILD_TEST)
  if(CORE_BUILD)
    include_directories(
      ${CMAKE_SOURCE_DIR}/bindings/d
      )
  else(CORE_BUILD)
    include_directories(${INCLUDE_DIR})
  endif(CORE_BUILD)

  set_property(GLOBAL PROPERTY TARGETS_examples_d)
  foreach(STRING_INDEX ${d_STRING_INDICES})
    add_executable(x${STRING_INDEX}d x${STRING_INDEX}d.d)
    target_link_libraries(x${STRING_INDEX}d PLPLOT::plplotdmd PLPLOT::plplot ${MATH_LIB})
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_d x${STRING_INDEX}d)
  endforeach(STRING_INDEX ${d_STRING_INDICES})
endif(BUILD_TEST)

