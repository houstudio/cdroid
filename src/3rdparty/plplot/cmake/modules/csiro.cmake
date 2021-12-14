# cmake/modules/csiro.cmake
#
# Copyright (C) 2006-2019  Alan W. Irwin
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

# Module for determining all configuration variables for the CSIRO libraries.

option(PL_HAVE_QHULL "Enable use of the Qhull library" ON)
option(WITH_CSA "Enable use of the csa library" ON)

# This logic copied verbatim from csiro.ac for ix86 systems and alpha systems
# with two possible compilers.  In future, this logic will need to be
# expanded to a lot more cases as we gain platform experience.
set(NAN_CFLAGS ${CMAKE_C_FLAGS})
if(PL_HAVE_QHULL OR WITH_CSA)
  if(CMAKE_SYSTEM_PROCESSOR MATCHES "i[0-9]86" AND NOT CMAKE_C_COMPILER MATCHES "gcc")
    set(NAN_CFLAGS "${NAN_CFLAGS} -mieee-fp")
  elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "alpha.*")
    if(CMAKE_C_COMPILER MATCHES "gcc")
      set(NAN_CFLAGS "${NAN_CFLAGS} -mieee")
    else(CMAKE_C_COMPILER MATCHES "gcc")
      set(NAN_CFLAGS "${NAN_CFLAGS} -ieee")
    endif(CMAKE_C_COMPILER MATCHES "gcc")
  endif(CMAKE_SYSTEM_PROCESSOR MATCHES "i[0-9]86" AND NOT CMAKE_C_COMPILER MATCHES "gcc")
  if(NOT DEFINED NaNAwareCCompiler)
    message(STATUS "Check for NaN awareness in C compiler")
    try_run(RUN_RESULT COMPILE_RESULT
      ${PROJECT_BINARY_DIR}
      ${PROJECT_SOURCE_DIR}/cmake/modules/TestNaNAware.c
      CMAKE_FLAGS "-DCOMPILE_DEFINITIONS:STRING=${NAN_CFLAGS} -I\"${PROJECT_SOURCE_DIR}\"/lib/csa ${MATH_LIB}"
      OUTPUT_VARIABLE OUTPUT
      )
    #message(STATUS "COMPILE_RESULT = ${COMPILE_RESULT}")
    #message(STATUS "RUN_RESULT = ${RUN_RESULT}")
    if(COMPILE_RESULT AND NOT RUN_RESULT MATCHES "FAILED_TO_RUN" AND RUN_RESULT)
      set(NaNAwareCCompiler ON)
    else(COMPILE_RESULT AND NOT RUN_RESULT MATCHES "FAILED_TO_RUN" AND RUN_RESULT)
      set(NaNAwareCCompiler OFF)
    endif(COMPILE_RESULT AND NOT RUN_RESULT MATCHES "FAILED_TO_RUN" AND RUN_RESULT)

    if(NaNAwareCCompiler)
      message(STATUS "Check for NaN awareness in C compiler - found")
      file(APPEND ${PROJECT_BINARY_DIR}/CMakeFiles/CMakeOutput.log
	"Determining whether C compiler is NaN aware succeeded with "
	"the following output:\n${OUTPUT}\n\n"
	)
    else(NaNAwareCCompiler)
      message(STATUS "Check for NaN awareness in C compiler - not found")
      message(STATUS "WARNING: Setting PL_HAVE_QHULL and WITH_CSA to OFF.")
      set(PL_HAVE_QHULL OFF CACHE BOOL "Enable use of the Qhull library" FORCE)
      set(WITH_CSA OFF CACHE BOOL "Enable use of the csa library" FORCE)
      file(APPEND ${PROJECT_BINARY_DIR}/CMakeFiles/CMakeError.log
	"Determining whether C compiler is NaN aware failed with "
	"the following output:\n${OUTPUT}\n\n"
	)
    endif(NaNAwareCCompiler)
  endif(NOT DEFINED NaNAwareCCompiler)
endif(PL_HAVE_QHULL OR WITH_CSA)

if(PL_HAVE_QHULL)
  find_package(QHULL)
  if(QHULL_FOUND)
    message(STATUS "QHULL_INCLUDE_DIRS = ${QHULL_INCLUDE_DIRS}")
    message(STATUS "HAS_LIBQHULL_INCLUDE = ${HAS_LIBQHULL_INCLUDE}")
    message(STATUS "QHULL_LIBRARIES = ${QHULL_LIBRARIES}")
    set(QHULL_RPATH ${QHULL_LIBRARY_DIRS})
    filter_rpath(QHULL_RPATH)
    message(STATUS "QHULL_RPATH = ${QHULL_RPATH}")
    include(CheckFunctionExists)
    cmake_push_check_state()
    list(APPEND CMAKE_REQUIRED_LIBRARIES ${QHULL_LIBRARIES})
    check_function_exists(qh_new_qhull QH_NEW_EXIST)
    cmake_pop_check_state()
    if(NOT QH_NEW_EXIST)
      message(STATUS "WARNING: function qh_new_qhull not found.  Setting PL_HAVE_QHULL to OFF.")
      set(PL_HAVE_QHULL OFF CACHE BOOL "Enable use of the Qhull library" FORCE)
    endif(NOT QH_NEW_EXIST)
    if(HAS_LIBQHULL_INCLUDE)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Check_realT_size.c "#include <libqhull/qhull_a.h>")
    else(HAS_LIBQHULL_INCLUDE)
      file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/Check_realT_size.c "#include <qhull/qhull_a.h>")
    endif(HAS_LIBQHULL_INCLUDE)
    file(APPEND ${CMAKE_CURRENT_BINARY_DIR}/Check_realT_size.c
      "
int main(void)
{
  if ( sizeof ( realT ) != sizeof ( double ) )
    return 1;
  else
    return 0;
}
"
      )
    try_run(RUN_RESULT COMPILE_RESULT
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_CURRENT_BINARY_DIR}/Check_realT_size.c
      CMAKE_FLAGS "-DCOMPILE_DEFINITIONS:STRING=-I\"${QHULL_INCLUDE_DIRS}\""
      OUTPUT_VARIABLE OUTPUT
      )
    #message(STATUS "Check_realT_size COMPILE_RESULT = ${COMPILE_RESULT}")
    #message(STATUS "Check_realT_size RUN_RESULT = ${RUN_RESULT}")
    if(NOT COMPILE_RESULT OR RUN_RESULT MATCHES "FAILED_TO_RUN")
      message(STATUS "Either could not compile or could not run code to check wrong size for realT in qhull library.  Dropping qhull librarydependency of PLplot.")
      set(PL_HAVE_QHULL OFF CACHE BOOL "Enable use of the Qhull library" FORCE)
    endif(NOT COMPILE_RESULT OR RUN_RESULT MATCHES "FAILED_TO_RUN")

    if(RUN_RESULT)
      message(STATUS "qhull library compiled with incorrect (non-double) size for realT.  Dropping qhull dependency of PLplot.")
      set(PL_HAVE_QHULL OFF CACHE BOOL "Enable use of the Qhull library" FORCE)
    else(RUN_RESULT)
      message(STATUS "qhull library compiled with correct (double) size for realT.")
    endif(RUN_RESULT)

  else(QHULL_FOUND)
    message(STATUS "QHULL_INCLUDE_DIRS = ${QHULL_INCLUDE_DIRS}")
    message(STATUS "QHULL_LIBRARIES = ${QHULL_LIBRARIES}")
    message(STATUS
      "WARNING: at least one of QHULL_INCLUDE_DIRS or QHULL_LIBRARIES is false so setting PL_HAVE_QHULL to OFF."
      )
    set(PL_HAVE_QHULL OFF CACHE BOOL "Enable use of the Qhull library" FORCE)
  endif(QHULL_FOUND)
endif(PL_HAVE_QHULL)
