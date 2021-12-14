#
# CMakeD - CMake module for D Language
#
# Copyright (c) 2007, Selman Ulug <selman.ulug@gmail.com>
#                     Tim Burrell <tim.burrell@gmail.com>
#
# All rights reserved.
#
# See Copyright.txt for details.
#
# Modified from CMake 2.6.5 CMakeTestCCompiler.cmake
# See http://www.cmake.org/HTML/Copyright.html for details
#

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected D compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.

IF(NOT CMAKE_D_COMPILER_WORKS)
  MESSAGE(STATUS "Check for working D compiler: ${CMAKE_D_COMPILER}")
  FILE(WRITE ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
    "int main(char[][] args)\n"
    "{return args.sizeof-1;}\n")
  STRING(REGEX MATCH "dmd" DMDTEST "${CMAKE_D_COMPILER}")
  IF(DMDTEST STREQUAL "dmd")
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_COMPILER_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	OUTPUT_VARIABLE OUTPUT)
    ELSE(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_COMPILER_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	CMAKE_FLAGS "-DLINK_LIBRARIES=${D_PATH}/lib/libphobos2.a"
	OUTPUT_VARIABLE OUTPUT)
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  ELSE(DMDTEST STREQUAL "dmd")
    TRY_COMPILE(CMAKE_D_COMPILER_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
      OUTPUT_VARIABLE OUTPUT)
  ENDIF(DMDTEST STREQUAL "dmd")
  SET(C_TEST_WAS_RUN 1)
ENDIF(NOT CMAKE_D_COMPILER_WORKS)

IF(NOT CMAKE_D_COMPILER_WORKS)
  MESSAGE(STATUS "Check for working D compiler: ${CMAKE_D_COMPILER} -- broken")
  message(STATUS "To force a specific D compiler set the DC environment variable")
  message(STATUS "    ie - export DC=\"/opt/dmd/bin/dmd\"")
  message(STATUS "If the D path is not set please use the D_PATH variable")
  message(STATUS "    ie - export D_PATH=\"/opt/dmd\"")
  FILE(APPEND ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if the D compiler works failed with "
    "the following output:\n${OUTPUT}\n\n")
  MESSAGE(FATAL_ERROR "The D compiler \"${CMAKE_D_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
ELSE(NOT CMAKE_D_COMPILER_WORKS)
  IF(C_TEST_WAS_RUN)
    MESSAGE(STATUS "Check for working D compiler: ${CMAKE_D_COMPILER} -- works")
    FILE(APPEND ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the D compiler works passed with "
      "the following output:\n${OUTPUT}\n\n")
  ENDIF(C_TEST_WAS_RUN)
  SET(CMAKE_D_COMPILER_WORKS 1 CACHE INTERNAL "")
  # re-configure this file CMakeDCompiler.cmake so that it gets
  # the value for CMAKE_SIZEOF_VOID_P
  # configure variables set in this file for fast reload later on
  # FIXME.  This is PLplot-specific location.  Other projects will use
  # a different location.
  IF(EXISTS ${PROJECT_SOURCE_DIR}/cmake/modules/language_support/cmake/CMakeDCompiler.cmake.in)
    CONFIGURE_FILE(${PROJECT_SOURCE_DIR}/cmake/modules/language_support/cmake/CMakeDCompiler.cmake.in
      ${CMAKE_PLATFORM_INFO_DIR}/CMakeDCompiler.cmake IMMEDIATE)
  ELSE(EXISTS ${PROJECT_SOURCE_DIR}/cmake/modules/language_support/cmake/CMakeDCompiler.cmake.in)
    CONFIGURE_FILE(${CMAKE_ROOT}/Modules/CMakeDCompiler.cmake.in
      ${CMAKE_PLATFORM_INFO_DIR}/CMakeDCompiler.cmake IMMEDIATE)
  ENDIF(EXISTS ${PROJECT_SOURCE_DIR}/cmake/modules/language_support/cmake/CMakeDCompiler.cmake.in)
ENDIF(NOT CMAKE_D_COMPILER_WORKS)

IF(NOT CMAKE_D_PHOBOS_WORKS)
  MESSAGE(STATUS "Check for working Phobos")
  FILE(WRITE ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
    "import std.stdio;\n"
    "int main(char[][] args)\n"
    "{ writefln(\"%s\", args[0]); return args.sizeof-1;}\n")
  IF(CMAKE_COMPILER_IS_GDC)
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_PHOBOS_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	#CMAKE_FLAGS "-DLINK_LIBRARIES=gphobos"
	OUTPUT_VARIABLE OUTPUT)
    ELSE(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_PHOBOS_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	#CMAKE_FLAGS "-DLINK_LIBRARIES=gphobos2"
	CMAKE_FLAGS "-DLINK_LIBRARIES=gphobos"
	OUTPUT_VARIABLE OUTPUT)
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  ELSE(CMAKE_COMPILER_IS_GDC)
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_PHOBOS_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	OUTPUT_VARIABLE OUTPUT)
    ELSE(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_PHOBOS_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	CMAKE_FLAGS "-DLINK_LIBRARIES=${D_PATH}/lib/libphobos.a"
	COMPILE_DEFINITIONS "-I${D_PATH}/include -I${D_PATH}/import"
	OUTPUT_VARIABLE OUTPUT)
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  ENDIF(CMAKE_COMPILER_IS_GDC)
  SET(C_TEST_WAS_RUN 1)
ENDIF(NOT CMAKE_D_PHOBOS_WORKS)

IF(NOT CMAKE_D_PHOBOS_WORKS)
  MESSAGE(STATUS "Check for working Phobos -- unavailable")
  FILE(APPEND ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if Phobos works failed with "
    "the following output:\n${OUTPUT}\n\n")
  #MESSAGE(FATAL_ERROR "Phobos does not work")
ELSE(NOT CMAKE_D_PHOBOS_WORKS)
  IF(C_TEST_WAS_RUN)
    MESSAGE(STATUS "Check for working Phobos -- works")
    FILE(APPEND ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if Phobos works passed with "
      "the following output:\n${OUTPUT}\n\n")
  ENDIF(C_TEST_WAS_RUN)
  SET(CMAKE_D_PHOBOS_WORKS 1 CACHE INTERNAL "")
ENDIF(NOT CMAKE_D_PHOBOS_WORKS)

IF(NOT CMAKE_D_TANGO_WORKS)
  MESSAGE(STATUS "Check for working Tango")
  FILE(WRITE ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
    "import tango.io.Stdout;"
    "int main(char[][] args)\n"
    "{Stdout.newline();return args.sizeof-1;}\n")
  IF(CMAKE_COMPILER_IS_GDC)
    TRY_COMPILE(CMAKE_D_TANGO_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
      CMAKE_FLAGS "-DLINK_LIBRARIES=gtango"
      OUTPUT_VARIABLE OUTPUT)
  ELSE(CMAKE_COMPILER_IS_GDC)
    IF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_TANGO_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	OUTPUT_VARIABLE OUTPUT)
    ELSE(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
      TRY_COMPILE(CMAKE_D_TANGO_WORKS ${PROJECT_BINARY_DIR} ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testDCompiler.d
	CMAKE_FLAGS "-DLINK_LIBRARIES=${D_PATH}/lib/libtango.a;${D_PATH}/lib/libphobos.a"
	COMPILE_DEFINITIONS "-I${D_PATH}/include -I${D_PATH}/import"
	OUTPUT_VARIABLE OUTPUT)
    ENDIF(${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
  ENDIF(CMAKE_COMPILER_IS_GDC)
  SET(C_TEST_WAS_RUN 1)
ENDIF(NOT CMAKE_D_TANGO_WORKS)

IF(NOT CMAKE_D_TANGO_WORKS)
  MESSAGE(STATUS "Check for working Tango -- unavailable")
  FILE(APPEND ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if Tango works failed with "
    "the following output:\n${OUTPUT}\n\n")
  #MESSAGE(FATAL_ERROR "Tango does not work: \n${OUTPUT}\n\n")
ELSE(NOT CMAKE_D_TANGO_WORKS)
  IF(C_TEST_WAS_RUN)
    MESSAGE(STATUS "Check for working Tango -- works")
    FILE(APPEND ${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if Tango works passed with "
      "the following output:\n${OUTPUT}\n\n")
  ENDIF(C_TEST_WAS_RUN)
  SET(CMAKE_D_TANGO_WORKS 1 CACHE INTERNAL "")
ENDIF(NOT CMAKE_D_TANGO_WORKS)

# if both tango and phobos are selected try to choose which one is available
IF(CMAKE_D_USE_TANGO AND CMAKE_D_USE_PHOBOS)
  MESSAGE(FATAL_ERROR "Tango AND Phobos selected, please choose one or the other!")
ENDIF(CMAKE_D_USE_TANGO AND CMAKE_D_USE_PHOBOS)

# ensure the user has the appropriate std lib available
IF(CMAKE_D_USE_TANGO AND NOT CMAKE_D_TANGO_WORKS)
  MESSAGE(FATAL_ERROR "Tango is required for this project, but it is not available!")
ENDIF(CMAKE_D_USE_TANGO AND NOT CMAKE_D_TANGO_WORKS)

IF(CMAKE_D_USE_PHOBOS AND NOT CMAKE_D_PHOBOS_WORKS)
  MESSAGE(FATAL_ERROR "Phobos is required for this project, but it is not available!")
ENDIF(CMAKE_D_USE_PHOBOS AND NOT CMAKE_D_PHOBOS_WORKS)

