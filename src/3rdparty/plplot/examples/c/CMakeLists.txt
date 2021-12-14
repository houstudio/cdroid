# examples/c/CMakeLists.txt
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

# N.B. This file is used for both the core build (which installs the examples
# and optionally [depending on BUILD_TEST} builds them) and the installed
# examples build.  The core build has BUILD_TEST OFF or ON at user option
# and CORE_BUILD always ON.  The installed examples build always has
# BUILD_TEST ON and CORE_BUILD OFF.

set(c_STRING_INDICES
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
  "32"
  "33"
  "34"
  )

if(CORE_BUILD)
  set(c_SRCS
    plcdemos.h
    tutor.c
    test_plend.c
    test_plbuf.c
    )
  foreach(STRING_INDEX ${c_STRING_INDICES})
    set(c_SRCS ${c_SRCS} x${STRING_INDEX}c.c)
  endforeach(STRING_INDEX ${c_STRING_INDICES})

  if(PLD_xcairo)
    set(extXdrawable_SRC extXdrawable_demo.c)
  endif(PLD_xcairo)

  if(PLD_extcairo)
    set(extcairo_SRC ext-cairo-test.c)
  endif(PLD_extcairo)

  if(PLD_xcairo OR PLD_extcairo)
    set(cairo_DOCS README.cairo)
  endif(PLD_xcairo OR PLD_extcairo)

  install(FILES ${c_SRCS} ${extXdrawable_SRC} ${extcairo_SRC} ${cairo_DOCS}
    DESTINATION ${DATA_DIR}/examples/c
    )

  install(FILES CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/c
    )

  set(CC "${CMAKE_C_COMPILER} ${CMAKE_C_COMPILER_ARG1} ${CMAKE_C_FLAGS}")

endif(CORE_BUILD)

if(BUILD_TEST)
  if(CORE_BUILD)
    include_directories(
      ${PROJECT_SOURCE_DIR}/include
      ${PROJECT_BINARY_DIR}/include
      ${PROJECT_SOURCE_DIR}/lib/qsastime
      ${PROJECT_BINARY_DIR}/lib/qsastime
      )
  else(CORE_BUILD)
    include_directories(${INCLUDE_DIR})
  endif(CORE_BUILD)

  set_property(GLOBAL PROPERTY TARGETS_examples_c)
  foreach(STRING_INDEX ${c_STRING_INDICES})
    add_executable(x${STRING_INDEX}c x${STRING_INDEX}c.c)
    if(BUILD_SHARED_LIBS)
      set_target_properties(x${STRING_INDEX}c PROPERTIES
	COMPILE_DEFINITIONS "USINGDLL"
	)
    endif(BUILD_SHARED_LIBS)
    target_link_libraries(x${STRING_INDEX}c PLPLOT::plplot ${MATH_LIB})
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_c x${STRING_INDEX}c)
  endforeach(STRING_INDEX ${c_STRING_INDICES})

  # Build simple test routines which nothing else depends on.
  add_executable(test_plend test_plend.c)
  if(BUILD_SHARED_LIBS)
    set_target_properties(test_plend PROPERTIES
      COMPILE_DEFINITIONS "USINGDLL"
      )
  endif(BUILD_SHARED_LIBS)
  target_link_libraries(test_plend PLPLOT::plplot ${MATH_LIB})

  # Build test routines that test the plot buffer functionality
  add_executable(test_plbuf test_plbuf.c)
  if(BUILD_SHARED_LIBS)
    set_target_properties(test_plbuf PROPERTIES
      COMPILE_DEFINITIONS "USINGDLL"
      )
  endif(BUILD_SHARED_LIBS)
  target_link_libraries(test_plbuf PLPLOT::plplot ${MATH_LIB})
endif(BUILD_TEST)

if(PKG_CONFIG_EXECUTABLE)
  # extXdrawable_demo is a gtk+ version 2 application.
  if(PLD_xcairo AND NOT DROP_GTK_PLUS_2_BUILDS)
    message(STATUS "Determine compile and link flags for extXdrawable_demo")
    pkg_check_pkgconfig(gtk+-x11-2.0 includedir libdir linkflags3 cflags3 version3 _GTK_X11)
    if(linkflags3)
      # Requires blank-delimited list.
      string(REGEX REPLACE ";" " " GTK_X11_COMPILE_FLAGS "${cflags3}")
      set(GTK_X11_LINK_FLAGS "${linkflags3}")
      if(BUILD_TEST)
	set_source_files_properties(
	  extXdrawable_demo.c
	  PROPERTIES COMPILE_FLAGS "${GTK_X11_COMPILE_FLAGS}"
	  )
	add_executable(extXdrawable_demo extXdrawable_demo.c)
	if(BUILD_SHARED_LIBS)
	  set_target_properties(extXdrawable_demo PROPERTIES
	    COMPILE_DEFINITIONS "USINGDLL"
	    )
	endif(BUILD_SHARED_LIBS)
	target_link_libraries(extXdrawable_demo ${GTK_X11_LINK_FLAGS} PLPLOT::plplot)
      endif(BUILD_TEST)
    else(linkflags3)
      message(STATUS "WARNING: gtk+-x11-2.0 not found.  extXdrawable_demo not built.")
      set(extXdrawable_true "#")
    endif(linkflags3)
  else(PLD_xcairo AND NOT DROP_GTK_PLUS_2_BUILDS)
    set(extXdrawable_true "#")
  endif(PLD_xcairo AND NOT DROP_GTK_PLUS_2_BUILDS)

  if(PLD_extcairo)
    message(STATUS "Determine compile and link flags for ext-cairo-test")
    pkg_check_pkgconfig(cairo includedir libdir linkflags4 cflags4 version4 _CAIRO)
    if(linkflags4)
      # Requires blank-delimited list.
      string(REGEX REPLACE ";" " " CAIRO_COMPILE_FLAGS "${cflags4}")
      set(CAIRO_LINK_FLAGS "${linkflags4}")
      if(BUILD_TEST)
	set_source_files_properties(
	  ext-cairo-test.c
	  PROPERTIES COMPILE_FLAGS "${CAIRO_COMPILE_FLAGS}"
	  )
	add_executable(ext-cairo-test ext-cairo-test.c)
	if(BUILD_SHARED_LIBS)
	  set_target_properties(ext-cairo-test PROPERTIES
	    COMPILE_DEFINITIONS "USINGDLL"
	    )
	endif(BUILD_SHARED_LIBS)
	target_link_libraries(ext-cairo-test ${CAIRO_LINK_FLAGS} PLPLOT::plplot)
      endif(BUILD_TEST)
    else(linkflags4)
      message(STATUS "WARNING: cairo not found.  ext-cairo-test not built.")
      set(extcairo_true "#")
    endif(linkflags4)
  else(PLD_extcairo)
    set(extcairo_true "#")
  endif(PLD_extcairo)
else(PKG_CONFIG_EXECUTABLE)
  message(STATUS "WARNING: pkg-config not found so extXdrawable_demo, and ext-cairo-test not built.")
  set(extXdrawable_true "#")
  set(extcairo_true "#")
endif(PKG_CONFIG_EXECUTABLE)

if(CORE_BUILD)
  # This configuration must occur after extXdrawable_true and extcairo_true are defined above.
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/c
    RENAME Makefile
    )
endif(CORE_BUILD)
