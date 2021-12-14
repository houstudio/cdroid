# lib/nistcd/CMakeLists.txt for PLplot
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

# build nistcd library (a.k.a., "cd" library put into the public domain by
# NIST) required by the PLplot cgm device.
if(PLD_cgm)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/cddll.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/cddll.h
    )

  set(cd_LIB_HDRS
    ${CMAKE_CURRENT_SOURCE_DIR}/cd.h
    ${CMAKE_CURRENT_SOURCE_DIR}/defines.h
    ${CMAKE_CURRENT_BINARY_DIR}/cddll.h
    )

  include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_BINARY_DIR}
  )

  set( cd_LIB_SRCS cd.c )

  # Bypass "#include <malloc.h>" in all code where it appears since
  # being too specific like this fails on some platforms (e.g., OS X), and
  # the proper form of the malloc.h include should be dragged in by
  # ""#include stdlib.h" in any case.
  # There may be some arcane platforms where we should not define this,
  # but for now, at least, try it for all platforms.
  set( cd_ADD_DEFINES "-DNOMALLOCH" )

  set_source_files_properties( ${cd_LIB_SRCS}
    PROPERTIES COMPILE_FLAGS "${cd_ADD_DEFINES}"
    )

  configure_library_build(nistcd "" "${cd_LIB_SRCS}" "" "")

  option(TEST_NISTCD "Test libnistcd" ON)
  if(TEST_NISTCD AND NOT CMAKE_CROSSCOMPILING)
    file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir)

    # build applications and configure tests for libnistcd.
    set( cd_EXECUTABLES
      cdexpert
      cdmulti
      cdsimple
      cdtest
      cdtext
      color16
      )

    foreach(cd_EXECUTABLE ${cd_EXECUTABLES})
      set_source_files_properties( ${cd_EXECUTABLE}.c
        PROPERTIES COMPILE_FLAGS "${cd_ADD_DEFINES}"
        )
      add_executable(${cd_EXECUTABLE} ${cd_EXECUTABLE}.c)
      if(BUILD_SHARED_LIBS)
	set_target_properties(${cd_EXECUTABLE} PROPERTIES
	  COMPILE_DEFINITIONS "USINGDLL"
	  )
      endif(BUILD_SHARED_LIBS)
      target_link_libraries( ${cd_EXECUTABLE} PLPLOT::nistcd )

      # Create tests for libnistcd.
      if(cd_EXECUTABLE STREQUAL "cdexpert")
	set(output_template cdexp1)
      elseif(cd_EXECUTABLE STREQUAL "cdtest")
	set(output_template cdout)
      else(cd_EXECUTABLE STREQUAL "cdexpert")
	set(output_template ${cd_EXECUTABLE})
      endif(cd_EXECUTABLE STREQUAL "cdexpert")

      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/${output_template}.cgm
	COMMAND $<TARGET_FILE:${cd_EXECUTABLE}>
	COMMAND ${CMAKE_COMMAND} -E compare_files
	${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/${output_template}.cgm
	${CMAKE_CURRENT_SOURCE_DIR}/${output_template}.cgm
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir
	DEPENDS
	${cd_EXECUTABLE}
	)

    endforeach(cd_EXECUTABLE ${cd_EXECUTABLES})

    add_custom_target(test_nistcd ALL
      DEPENDS
      ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/cdexp1.cgm
      ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/cdmulti.cgm
      ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/cdsimple.cgm
      ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/cdout.cgm
      ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/cdtext.cgm
      ${CMAKE_CURRENT_BINARY_DIR}/test_nistcd_dir/color16.cgm
      )

  endif(TEST_NISTCD AND NOT CMAKE_CROSSCOMPILING)

  # install library header files.
  install(FILES ${cd_LIB_HDRS} DESTINATION ${INCLUDE_DIR})
endif(PLD_cgm)
