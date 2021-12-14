
#=============================================================================
# Copyright 2003-2012 Kitware, Inc.
# Copyright 2015 Alan W. Irwin
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
#   notice, this list of conditions and the following disclaimer.
#
# * Redistributions in binary form must reproduce the above copyright
#   notice, this list of conditions and the following disclaimer in the
#   documentation and/or other materials provided with the distribution.
#
# * Neither the names of Kitware, Inc., the Insight Software Consortium,
#   nor the names of their contributors may be used to endorse or promote
#   products derived from this software without specific prior written
#   permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#=============================================================================

if(CMAKE_Ada_COMPILER_FORCED)
  # The compiler configuration was forced by the user.
  # Assume the user has configured all compiler information.
  set(CMAKE_Ada_COMPILER_WORKS TRUE)
  return()
endif()

include(CMakeTestCompilerCommon)

# Remove any cached result from an older CMake version.
# We now store this in CMakeAdaCompiler.cmake.
unset(CMAKE_Ada_COMPILER_WORKS CACHE)

# This file is used by EnableLanguage in cmGlobalGenerator to
# determine that that selected Ada compiler can actually compile
# and link the most basic of programs.   If not, a fatal error
# is set and cmake stops processing commands and will not generate
# any makefiles or projects.
if(NOT CMAKE_Ada_COMPILER_WORKS)
  PrintTestCompilerStatus("${GNAT_EXECUTABLE_BUILDER}" "")
  file(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/testadacompiler.adb
    "
procedure testAdaCompiler is
begin
  null;
end;
  "
    )

  # Must use "whole project" form of try_compile because the "compile
  # file" form is too generic to be used for Ada compilation of an
  # executable which has special requirements with regard to source
  # file must be named target.adb.
  FILE(WRITE ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp/CMakeLists.txt
  "
# MAINTENANCE
# Use same minimum version for all platforms as the Linux platform minimum adopted for
# the PLplot project.
cmake_minimum_required(VERSION 3.7.2 FATAL_ERROR)
set(CMAKE_MODULE_PATH \"${CMAKE_Ada_MODULES}\")
set(CMAKE_VERBOSE_MAKEFILE ON CACHE BOOL \"\" FORCE)
project(test Ada)
add_executable(testadacompiler testadacompiler.adb)
  "
  )

# To avoid try_compile recurse error, use any binary directory other than
# ${CMAKE_BINARY_DIR}.  The choice of
# bindir = ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp is
# especially advantageous since it makes an in-source build which
# means that no special variables need to be set to find files.
  try_compile(CMAKE_Ada_COMPILER_WORKS
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp
  ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeTmp
  projectName
  OUTPUT_VARIABLE __CMAKE_Ada_COMPILER_OUTPUT
  )
  SET(ADA_TEST_WAS_RUN 1)

  # Move result from cache to normal variable.
  set(CMAKE_Ada_COMPILER_WORKS ${CMAKE_Ada_COMPILER_WORKS})
  unset(CMAKE_Ada_COMPILER_WORKS CACHE)
  set(Ada_TEST_WAS_RUN 1)
endif()

if(NOT CMAKE_Ada_COMPILER_WORKS)
  PrintTestCompilerStatus("${GNAT_EXECUTABLE_BUILDER}" " -- broken")
  file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeError.log
    "Determining if the Ada compiler works failed with "
    "the following output:\n${__CMAKE_Ada_COMPILER_OUTPUT}\n\n")
  message(FATAL_ERROR "The Ada compiler \"${CMAKE_Ada_COMPILER}\" "
    "is not able to compile a simple test program.\nIt fails "
    "with the following output:\n ${__CMAKE_Ada_COMPILER_OUTPUT}\n\n"
    "CMake will not be able to correctly generate this project.")
else()
  if(ADA_TEST_WAS_RUN)
    PrintTestCompilerStatus("${GNAT_EXECUTABLE_BUILDER}" " -- works")
    file(APPEND ${CMAKE_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/CMakeOutput.log
      "Determining if the Ada compiler works passed with "
      "the following output:\n${__CMAKE_Ada_COMPILER_OUTPUT}\n\n")
  endif()

  # Re-configure to save learned information.
  configure_file(
    ${CMAKE_Ada_MODULES}/CMakeAdaCompiler.cmake.in
    ${CMAKE_PLATFORM_INFO_DIR}/CMakeAdaCompiler.cmake
    @ONLY
    )
  include(${CMAKE_PLATFORM_INFO_DIR}/CMakeAdaCompiler.cmake)

endif()

unset(__CMAKE_Ada_COMPILER_OUTPUT)
