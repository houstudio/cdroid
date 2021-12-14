# examples/ada/CMakeLists.txt
# Copyright (C) 2007-2018 Alan W. Irwin
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

set(ada_STRING_INDICES
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
  set(ada_SRCS)
  foreach(STRING_INDEX ${ada_STRING_INDICES})
    set(SOURCE_FILE xtraditional${STRING_INDEX}a.adb)
    list(APPEND ada_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
    set(SOURCE_FILE xstandard${STRING_INDEX}a.adb)
    list(APPEND ada_SRCS ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})
  endforeach(STRING_INDEX ${ada_STRING_INDICES})

  install(FILES ${ada_SRCS} DESTINATION ${DATA_DIR}/examples/ada)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/ada
    RENAME Makefile
    )

  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/ada
    )
endif(CORE_BUILD)

if(BUILD_TEST)
  # Setup compilation of nominal source file (see below) by including
  # directory where test_ada library spec (*.ads) files reside.
  if(CORE_BUILD)
    include_directories(
      ${CMAKE_SOURCE_DIR}/bindings/ada
      )
  else(CORE_BUILD)
    include_directories(${ADA_INCLUDE_DIR})
  endif(CORE_BUILD)

  # The first -aI option in adalinkflags is required to find
  # <TARGET>.adb (see comment below). The second -aI option gives
  # access to the libplplotada spec (*.ads) files. The -aL option
  # gives access to the libplplotada *.ali files.  (For the CORE_BUILD
  # case that location currently works but is probably not guaranteed
  # indefinitely for future versions of CMake.)

  if(CORE_BUILD)
    set(adalinkflags "\"-aI${CMAKE_CURRENT_SOURCE_DIR}\" \"-aI${CMAKE_SOURCE_DIR}/bindings/ada\" \"-aL${CMAKE_BINARY_DIR}/bindings/ada/CMakeFiles/plplotada.dir\"")
  else(CORE_BUILD)
    set(adalinkflags "\"-aI${CMAKE_CURRENT_SOURCE_DIR}\" \"-aI${ADA_INCLUDE_DIR}\" \"-aL${ADA_LIB_DIR}\"")
  endif(CORE_BUILD)

  set(ada_extra_clean_files)
  foreach(STRING_INDEX ${ada_STRING_INDICES})
    foreach(BINDING traditional standard)
      set(TARGET_NAME x${BINDING}${STRING_INDEX}a)
      set(SOURCE_FILE ${TARGET_NAME}.adb)
      set(adasrcfile ${CMAKE_CURRENT_SOURCE_DIR}/${SOURCE_FILE})

      # Note due to limitations of the Ada language support for CMake,
      # the nominal source file for add_executable is compiled (which
      # is why a real file has to be specified), but otherwise it is
      # ignored except for its .adb suffix which identifies the Ada
      # language.  The actual source file name for the internal
      # gnatmake command that creates the executable is constructed
      # from <TARGET>.adb.  Since no directory can be specified from
      # this construction (don't ask), you must specify the directory
      # of <TARGET>.adb with an -aI option (see above comment of
      # adalinkflags).
      add_executable(${TARGET_NAME} ${adasrcfile})

      set_target_properties(${TARGET_NAME} PROPERTIES LINK_FLAGS ${adalinkflags})
      target_link_libraries(${TARGET_NAME} PLPLOT::plplotada PLPLOT::plplot)
      set_property(GLOBAL APPEND PROPERTY TARGETS_examples_ada ${TARGET_NAME})

      # CMake assumes compilation results only in object files being generated.
      # However, gnatmake generates both object files and *.ali (Ada library
      # information) files so it doesn't intrinsically know how to clean those
      # additional *.ali files.
      # Here is a workaround for this fundamental CMake limitation.

      # Add generated .ali file and "extra" generated pair of .ali and .o files
      # to the list of additional files to be removed by make clean
      list(APPEND ada_extra_clean_files "${CMAKE_CURRENT_BINARY_DIR}/CMakeFiles/${TARGET_NAME}.dir/${TARGET_NAME}.ali")
      list(APPEND ada_extra_clean_files "${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.o;${CMAKE_CURRENT_BINARY_DIR}/${TARGET_NAME}.ali")
    endforeach(BINDING traditional standard)
  endforeach(STRING_INDEX ${ada_STRING_INDICES})

  set_directory_properties(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "${ada_extra_clean_files}")
endif(BUILD_TEST)
