# cmake/modules/TestFortranIsnan.cmake
#
# Fortran binding - check is isnan function is available
#
# Copyright (C) 2008  Andrew Ross
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

# Check if isnan function is available in fortran
IF(NOT DEFINED CMAKE_FORTRAN_HAVE_ISNAN)
  MESSAGE(STATUS "Check if isnan function is available in fortran")
  TRY_COMPILE(CMAKE_FORTRAN_HAVE_ISNAN
    ${CMAKE_BINARY_DIR}
    ${CMAKE_SOURCE_DIR}/cmake/modules/TestFortranIsnan.f
    OUTPUT_VARIABLE OUTPUT)
  IF (CMAKE_FORTRAN_HAVE_ISNAN)
    MESSAGE(STATUS "Check for isnan in fortran - found")
    SET (FORTRAN_HAVE_ISNAN 1 CACHE INTERNAL
      "Does fortran have isnan function")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeOutput.log
      "Determining if fortran has isnan function passed with "
      "the following output:\n${OUTPUT}\n\n")
  ELSE (CMAKE_FORTRAN_HAVE_ISNAN)
    MESSAGE(STATUS "Check for isnan in fortran - not found")
    SET (FORTRAN_HAVE_ISNAN 0 CACHE INTERNAL
      "Does fortran have isnan function")
      FILE(APPEND ${CMAKE_BINARY_DIR}/CMakeFiles/CMakeError.log
      "Determining if fortran has isnan function failed with "
      "the following output:\n${OUTPUT}\n\n")
  ENDIF (CMAKE_FORTRAN_HAVE_ISNAN)
ENDIF(NOT DEFINED CMAKE_FORTRAN_HAVE_ISNAN)
