# examples/java/CMakeLists.txt
### Process this file with cmake to produce Makefile
###
# Copyright (C) 2006-2015 Alan W. Irwin
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

set(java_STRING_INDICES
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
  set(java_SRCS)
  foreach(STRING_INDEX ${java_STRING_INDICES})
    list(APPEND java_SRCS x${STRING_INDEX}.java)
  endforeach(STRING_INDEX ${java_STRING_INDICES})

  install(FILES ${java_SRCS} README.javademos
    DESTINATION ${DATA_DIR}/examples/java
    )

  set(JAVAC ${CMAKE_Java_COMPILER})
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/java
    RENAME Makefile
    )

  install(FILES CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/java
    )

  # Build all java examples regardless of BUILD_test to allow putting
  # the class files in the jar file that will be created and installed.
  set(java_CLASSES)
  get_property(files_plplot_core GLOBAL PROPERTY FILES_plplot_core)
  set_property(GLOBAL PROPERTY TARGETS_examples_java)
  set_property(GLOBAL PROPERTY FILES_examples_java)
  foreach(STRING_INDEX ${java_STRING_INDICES})
    set(in_file ${CMAKE_CURRENT_SOURCE_DIR}/x${STRING_INDEX}.java)
    set(out_file
      ${CMAKE_CURRENT_BINARY_DIR}/plplot/examples/x${STRING_INDEX}.class
      )
    list(APPEND java_CLASSES ${out_file})
    add_custom_command(
      OUTPUT ${out_file}
      COMMAND ${CMAKE_Java_COMPILER}
      ${JAVACFLAGS} -classpath ${CMAKE_BINARY_DIR}/bindings/java -d ${CMAKE_CURRENT_BINARY_DIR} -encoding UTF-8 ${in_file}
      DEPENDS ${in_file} ${files_plplot_core}
      VERBATIM
      )
    add_custom_target(x${STRING_INDEX}j ALL DEPENDS ${out_file})
    add_dependencies(x${STRING_INDEX}j plplot_core)
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_java x${STRING_INDEX}j)
    set_property(GLOBAL APPEND PROPERTY FILES_examples_java ${out_file})
  endforeach(STRING_INDEX ${java_STRING_INDICES})

  add_custom_target(plplot_examples DEPENDS ${java_CLASSES})
  get_property(targets_examples_java GLOBAL PROPERTY TARGETS_examples_java)
  #message(STATUS "targets_examples_java = ${targets_examples_java}")
  # The ${targets_examples_java} dependency is required to avoid a
  # race condition for creating the files in ${java_CLASSES} for
  # parallel builds.
  add_dependencies(plplot_examples plplot_core ${targets_examples_java})

  # This information is duplicated from bindings/java/CMakeLists.txt.
  # It is required here to get the dependencies right for
  # the jar file.
  set(java_CORE_CLASSES
    ${CMAKE_BINARY_DIR}/bindings/java/plplot/core/PLGraphicsIn.class
    ${CMAKE_BINARY_DIR}/bindings/java/plplot/core/PLStream.class
    ${CMAKE_BINARY_DIR}/bindings/java/plplot/core/config.class
    ${CMAKE_BINARY_DIR}/bindings/java/plplot/core/plplotjavac.class
    ${CMAKE_BINARY_DIR}/bindings/java/plplot/core/plplotjavacConstants.class
    ${CMAKE_BINARY_DIR}/bindings/java/plplot/core/plplotjavacJNI.class
    )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/plplot.jar
    COMMAND ${CMAKE_Java_ARCHIVE}
    -cf ${CMAKE_CURRENT_BINARY_DIR}/plplot.jar -C ${CMAKE_BINARY_DIR}/bindings/java plplot/core -C ${CMAKE_CURRENT_BINARY_DIR} plplot/examples
    DEPENDS ${java_CLASSES} ${java_CORE_CLASSES}
    )

  add_custom_target(plplot_jar ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/plplot.jar)

  # The plplot_examples and ${targets_examples_java} dependency is
  # required to avoid a race condition for creating the files in
  # ${java_CLASSES} for parallel builds.

  add_dependencies(plplot_jar plplot_examples ${targets_examples_java} plplot_core)
  set_property(GLOBAL APPEND PROPERTY TARGETS_examples_java plplot_jar)
  set_property(GLOBAL APPEND PROPERTY FILES_examples_java ${CMAKE_CURRENT_BINARY_DIR}/plplot.jar)

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/plplot.jar
    DESTINATION ${JAR_DIR}
    )
else(CORE_BUILD)
  set(INSTALLED_JAR_FILE ${JAR_DIR}/plplot.jar)
  set_property(GLOBAL PROPERTY TARGETS_examples_java)
  set_property(GLOBAL PROPERTY FILES_examples_java)
  foreach(STRING_INDEX ${java_STRING_INDICES})
    set(in_file ${CMAKE_CURRENT_SOURCE_DIR}/x${STRING_INDEX}.java)
    set(
      out_file
      ${CMAKE_CURRENT_BINARY_DIR}/plplot/examples/x${STRING_INDEX}.class
      )
    add_custom_command(
      OUTPUT ${out_file}
      COMMAND ${CMAKE_Java_COMPILER}
      ${JAVACFLAGS} -classpath ${INSTALLED_JAR_FILE} -d ${CMAKE_CURRENT_BINARY_DIR} -encoding UTF-8 ${in_file}
      DEPENDS ${in_file} ${INSTALLED_JAR_FILE}
      VERBATIM
      )
    add_custom_target(x${STRING_INDEX}j ALL DEPENDS ${out_file})
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_java x${STRING_INDEX}j)
    set_property(GLOBAL APPEND PROPERTY FILES_examples_java ${out_file})
  endforeach(STRING_INDEX ${java_STRING_INDICES})
endif(CORE_BUILD)
