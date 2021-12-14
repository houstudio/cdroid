# examples/fortran/CMakeLists.txt

# Copyright (C) 2006-2011 Andrew Ross
# Copyright (C) 2006-2019 Alan W. Irwin
# Copyright (C) 2008-2016 Arjen Markus
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

set(fortran_STRING_INDICES
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
  "16a"
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

# plfortrandemos.inc, which is included by x21f.f90, is located
# in ${CMAKE_CURRENT_BINARY_DIR} for the core build tree and
# in ${CMAKE_CURRENT_SOURCE_DIR} for the installed examples.
# Note, some Fortran compilers (e.g., nagfor) by default do not look for included
# files in the location of the source file when the build is occurring in
# a different directory.  To solve this issue in general
# always specify the location of plfortrandemos.inc using
# ${CMAKE_CURRENT_BINARY_DIR} or ${CMAKE_CURRENT_SOURCE_DIR}
# as appropriate.
if(CORE_BUILD)
  # The examples use either "use plfortrandemolib" or "use plplot".

  # Location of the module file for plfortrandemolib has changed now to the parent directory of this one.
  # See chicken/egg comment below.
  include_directories(${CMAKE_BINARY_DIR}/examples)

  # The module files for the fortran binding are created by
  # default during the library build in the bindings/fortran directory.
  include_directories(${CMAKE_BINARY_DIR}/bindings/fortran)

  # Location of plfortrandemos.inc.
  include_directories(${CMAKE_CURRENT_BINARY_DIR})
else(CORE_BUILD)
  # locations of installed fortran interface library, installed
  # fortran modules, and installed plfortrandemos.inc.
  include_directories(${INCLUDE_DIR} ${FORTRAN_MOD_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
endif(CORE_BUILD)

if(CORE_BUILD)
  # plfortrandemolib depends on the plplotfortran library.
  # The steps below which normally would be done in this directory are
  # skipped because they must be done instead in the parent directory
  # to solve a chicken/egg issue.
  if(0)
  set(LIB_INSTALL_RPATH ${LIB_DIR})
  configure_library_build(plfortrandemolib STATIC plfortrandemolib.f90 PLPLOT::plplotfortran "${LIB_INSTALL_RPATH}")
  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/plfortrandemolib.mod
    DESTINATION ${FORTRAN_MOD_DIR}
    )
  endif(0)

  set(fortran_SRCS)
  foreach(STRING_INDEX ${fortran_STRING_INDICES})
    set(fortran_SRCS ${fortran_SRCS} x${STRING_INDEX}f.f90)
  endforeach(STRING_INDEX ${fortran_STRING_INDICES})

  # Process fortran header file to define isnan
  if(FORTRAN_HAVE_ISNAN)
    set(HAVE_FORTRAN_ISNAN_FALSE !)
  else(FORTRAN_HAVE_ISNAN)
    set(HAVE_FORTRAN_ISNAN_TRUE !)
  endif(FORTRAN_HAVE_ISNAN)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plfortrandemos.inc.in
    ${CMAKE_CURRENT_BINARY_DIR}/plfortrandemos.inc
    )
  set_source_files_properties(${CMAKE_CURRENT_BINARY_DIR}/plfortrandemos.inc
    PROPERTIES GENERATED ON)

  install(FILES ${fortran_SRCS} README_precision
    DESTINATION ${DATA_DIR}/examples/fortran
    )

  set(FC "${CMAKE_Fortran_COMPILER} ${CMAKE_Fortran_COMPILER_ARG1} ${CMAKE_Fortran_FLAGS}")
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/fortran
    RENAME Makefile
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plfortrandemos.inc
    DESTINATION ${DATA_DIR}/examples/fortran
    )

  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/fortran
    )
endif(CORE_BUILD)

if(BUILD_TEST)

  set_property(GLOBAL PROPERTY TARGETS_examples_fortran)
  # nm analysis shows only this library is required to link examples to the new Fortran binding
  set(fortran_library_target PLPLOT::plplotfortran)
  foreach(STRING_INDEX ${fortran_STRING_INDICES})
    # Need unique target name
    add_executable(x${STRING_INDEX}f x${STRING_INDEX}f.f90)
    target_link_libraries(x${STRING_INDEX}f PLPLOT::plfortrandemolib ${fortran_library_target})
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_fortran x${STRING_INDEX}f)
  endforeach(STRING_INDEX ${fortran_STRING_INDICES})

  if(CORE_BUILD)
    # Build code that tests auxiliary functions in plfortrandemolib
    add_executable(test_plfortrandemolib test_plfortrandemolib.f90)
    target_link_libraries(test_plfortrandemolib PLPLOT::plfortrandemolib ${fortran_library_target})
  endif(CORE_BUILD)
endif(BUILD_TEST)
