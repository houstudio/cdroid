# plplot_test/CMakeLists.txt for PLplot
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

# Only configure and install PLplot test files and configure ctest tests if
# a shell is available since our test environment completely depends on a shell
# being available.
if(SH_EXECUTABLE)

  if(CORE_BUILD)
    # Configure and/or install PLplot test files.
    if(MSVC)
      set (VC_CTEST_DIRECTORY /$<CONFIGURATION>)
    endif(MSVC)
    if(CYGWIN)
      set(PLATFORM CYGWIN)
    elseif(MSYS)
      set(PLATFORM MSYS)
    elseif(UNIX)
      set(PLATFORM UNIX)
    elseif(MINGW)
      set(PLATFORM MINGW)
    elseif(MSVC)
      set(PLATFORM MSVC)
    else(CYGWIN)
      message(FATAL_ERROR "Internal build-system error; unable to determine platform.")
    endif(CYGWIN)

    # Part 1. Configure PLplot test files where the result is not
    # installed, but which instead is used just for the CMake build
    # tree of the core build.
    configure_file(
      plplot-test.sh.in
      ${CMAKE_CURRENT_BINARY_DIR}/plplot-test.sh
      @ONLY
      NEWLINE_STYLE UNIX
      )

    if(ENABLE_tkX)
      # N.B. these locations only used in the configured file
      # for the case where ENABLE_tkX is ON.  Will have to
      # change this logic if the script changes in this regard.
      set(pltcl_LOCATION $<TARGET_FILE:PLPLOT::pltcl>)
      set(plserver_LOCATION $<TARGET_FILE:PLPLOT::plserver>)
    endif(ENABLE_tkX)
    configure_file_generate(
      plplot-test-interactive.sh.in
      ${CMAKE_CURRENT_BINARY_DIR}/plplot-test-interactive.sh
      @ONLY
      NEWLINE_STYLE UNIX
      )

    if(ENABLE_tcl)
      set(PLTCL_DIR $<TARGET_FILE_DIR:PLPLOT::pltcl>)
      set(pltcl_LOCATION $<TARGET_FILE:PLPLOT::pltcl>)
      # N.B. Results only available at generate time.
      configure_file_generate(
	${CMAKE_CURRENT_SOURCE_DIR}/test_tcl.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_tcl.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
    endif(ENABLE_tcl)

    # Part 2. Install this CMakeLists.txt file.
    install(FILES CMakeLists.txt DESTINATION ${DATA_DIR}/examples/plplot_test)

    # Part 3. Configure AND install PLplot test files.
    set(PERM_SCRIPTS
      OWNER_READ
      OWNER_WRITE
      OWNER_EXECUTE
      GROUP_READ
      GROUP_EXECUTE
      WORLD_READ
      WORLD_EXECUTE
      )

    # Install-tree configuration of plplot-test.sh and plplot-test-interactive.sh
    configure_file(
      plplot-test.sh.in
      ${CMAKE_CURRENT_BINARY_DIR}/plplot-test.sh_install
      @ONLY
      NEWLINE_STYLE UNIX
      )
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/plplot-test.sh_install
      DESTINATION ${DATA_DIR}/examples
      PERMISSIONS ${PERM_SCRIPTS}
      RENAME plplot-test.sh
      )

    set(pltcl_LOCATION ${BIN_DIR}/pltcl)
    set(plserver_LOCATION ${BIN_DIR}/plserver)
    configure_file(
      plplot-test-interactive.sh.in
      ${CMAKE_CURRENT_BINARY_DIR}/plplot-test-interactive.sh_install
      @ONLY
      NEWLINE_STYLE UNIX
      )
    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/plplot-test-interactive.sh_install
      DESTINATION ${DATA_DIR}/examples
      PERMISSIONS ${PERM_SCRIPTS}
      RENAME plplot-test-interactive.sh
      )
    # Set up optional tests in the configured plplot-test-interactive.sh file
    # which is used by the traditional build system for the installed
    # examples to do some interactive testing.

    set(pkgIndex_LOCATION ${DATA_DIR})
    set(SHELL_TEST_PLTCL_STANDARD_EXAMPLES 1)
    set(SHELL_TEST_PLSERVER_STANDARD_EXAMPLES 1)
    # FIXME.  Currently plserver_runAllDemos needs work so drop it for now.
    set(SHELL_TEST_PLSERVER_RUNALLDEMOS 0)

    # All Tcl/Tk tests that use the "package require" approach only
    # work with shared libraries.
    if(BUILD_SHARED_LIBS)
      set(SHELL_TEST_TCLSH_STANDARD_EXAMPLES 1)
      # FIXME.  Currently wish_standard_examples segfaults on exit.  So
      # drop it for now.
      set(SHELL_TEST_WISH_STANDARD_EXAMPLES 0)
      # FIXME.  Currently the wish_runAllDemos needs work and also has
      # an intermittent bug (x?? not recognized as a command).
      set(SHELL_TEST_WISH_RUNALLDEMOS 0)
    else(BUILD_SHARED_LIBS)
      set(SHELL_TEST_TCLSH_STANDARD_EXAMPLES 0)
      set(SHELL_TEST_WISH_STANDARD_EXAMPLES 0)
      set(SHELL_TEST_WISH_RUNALLDEMOS 0)
    endif(BUILD_SHARED_LIBS)

    # Set this value to 0 if you want to temporarily disable wxwidgets tests
    # for the traditional build.
    set(SHELL_TEST_WXWIDGETS 1)

    # Set this value to 0 if you want to temporarily disable qt_example tests
    # for the traditional build.
    if(PLPLOT_USE_QT5)
      # FIXME once the Qt5 memory management issues are fixed
      set(SHELL_TEST_QT_EXAMPLE 0)
    else(PLPLOT_USE_QT5)
      set(SHELL_TEST_QT_EXAMPLE 1)
    endif(PLPLOT_USE_QT5)

    set(SCRIPTS)

    configure_file(
      test_c.sh.in
      ${CMAKE_CURRENT_BINARY_DIR}/test_c.sh
      @ONLY
      NEWLINE_STYLE UNIX
      )
    list(APPEND SCRIPTS test_c.sh)

    configure_file(
      test_c_interactive.sh.in
      ${CMAKE_CURRENT_BINARY_DIR}/test_c_interactive.sh
      @ONLY
      NEWLINE_STYLE UNIX
      )
    list(APPEND SCRIPTS test_c_interactive.sh)

    if(ENABLE_cxx)
      configure_file(
	test_cxx.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_cxx.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_cxx.sh)
    endif(ENABLE_cxx)

    if(ENABLE_fortran)
      configure_file(
	test_fortran.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_fortran.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_fortran.sh)
    endif(ENABLE_fortran)

    if(ENABLE_java)
      configure_file(
	test_java.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_java.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_java.sh)
    endif(ENABLE_java)

    if(ENABLE_octave)
      configure_file(
	test_octave_interactive.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_octave_interactive.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_octave_interactive.sh)

      configure_file(
	test_octave.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_octave.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_octave.sh)
    endif(ENABLE_octave)

    if(ENABLE_python)
      configure_file(
	test_python.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_python.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_python.sh)
    endif(ENABLE_python)

    if(ENABLE_tcl)
      set(PLTCL_DIR ${BIN_DIR})
      set(pltcl_LOCATION ${BIN_DIR}/pltcl)
      configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/test_tcl.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_tcl.sh_install
	@ONLY
	NEWLINE_STYLE UNIX
	)
      install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/test_tcl.sh_install
	DESTINATION ${DATA_DIR}/examples
	PERMISSIONS ${PERM_SCRIPTS}
	RENAME test_tcl.sh
	)
    endif(ENABLE_tcl)

    if(ENABLE_ada)
      configure_file(
	test_ada.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_ada.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_ada.sh)
    endif(ENABLE_ada)

    if(ENABLE_ocaml)
      configure_file(
	test_ocaml.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_ocaml.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_ocaml.sh)
    endif(ENABLE_ocaml)

    if(ENABLE_lua)
      configure_file(
	test_lua.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_lua.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_lua.sh)
    endif(ENABLE_lua)

    if(ENABLE_d)
      configure_file(
	test_d.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_d.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_d.sh)
    endif(ENABLE_d)

    if(PLD_plmeta)
      set(plrender_LOCATION $<TARGET_FILE:PLPLOT::plrender>)
      configure_file_generate(
	test_plrender.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_plrender.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_plrender.sh)

      set(plrender_LOCATION ${BIN_DIR}/plrender)
      configure_file(
	test_plrender.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_plrender.sh_install
	@ONLY
	NEWLINE_STYLE UNIX
	)
      install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/test_plrender.sh_install
	DESTINATION ${DATA_DIR}/examples
	PERMISSIONS ${PERM_SCRIPTS}
	RENAME test_plrender.sh
	)
    endif(PLD_plmeta)

    if(TEST_DIFF)
      configure_file(
	test_diff.sh.in
	${CMAKE_CURRENT_BINARY_DIR}/test_diff.sh
	@ONLY
	NEWLINE_STYLE UNIX
	)
      list(APPEND SCRIPTS test_diff.sh)
    endif(TEST_DIFF)

    foreach(SCRIPT ${SCRIPTS})
      install(
	FILES ${CMAKE_CURRENT_BINARY_DIR}/${SCRIPT}
	DESTINATION ${DATA_DIR}/examples
	PERMISSIONS ${PERM_SCRIPTS}
	)
    endforeach(SCRIPT ${SCRIPTS})

  endif(CORE_BUILD)

  # Part 4. Configure ctest tests.
  set(CTEST_EXAMPLES_OUTPUT_DIR ${CMAKE_BINARY_DIR}/ctest_examples_output_dir)
  file(MAKE_DIRECTORY ${CTEST_EXAMPLES_OUTPUT_DIR})

  string(REPLACE " " "\\ " CMAKE_BINARY_DIR_ESCAPED ${CMAKE_BINARY_DIR})
  string(REPLACE " " "\\ " CMAKE_SOURCE_DIR_ESCAPED ${CMAKE_SOURCE_DIR})
  string(REPLACE " " "\\ " CTEST_EXAMPLES_OUTPUT_DIR_ESCAPED ${CTEST_EXAMPLES_OUTPUT_DIR})
  if(CORE_BUILD)
    set(TEST_ENVIRONMENT
      "EXAMPLES_PREFIX=${CMAKE_BINARY_DIR_ESCAPED}/examples SRC_EXAMPLES_PREFIX=${CMAKE_SOURCE_DIR_ESCAPED}/examples OUTPUT_DIR=${CTEST_EXAMPLES_OUTPUT_DIR_ESCAPED} VC_CTEST_DIRECTORY=${VC_CTEST_DIRECTORY}"
      )
    set(TEST_SCRIPT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
  else(CORE_BUILD)
    set(TEST_ENVIRONMENT
      "EXAMPLES_PREFIX=${CMAKE_BINARY_DIR_ESCAPED} SRC_EXAMPLES_PREFIX=${CMAKE_SOURCE_DIR_ESCAPED} OUTPUT_DIR=${CTEST_EXAMPLES_OUTPUT_DIR_ESCAPED} VC_CTEST_DIRECTORY=${VC_CTEST_DIRECTORY}"
      )
    set(TEST_SCRIPT_DIRECTORY ${CMAKE_SOURCE_DIR})
  endif(CORE_BUILD)

  set(TEST_SCRIPT "./plplot-test.sh --verbose")
  message(STATUS "TEST_ENVIRONMENT = ${TEST_ENVIRONMENT}")
  set(PLD_TEST_DEVICE ${PLD_${PLPLOT_TEST_DEVICE}})

  set(TEST_SCRIPT_DEVICE "${TEST_SCRIPT} --device=${PLPLOT_TEST_DEVICE}")

  set(examples_compare_DEPENDS)

  if(PLD_TEST_DEVICE)
    add_test(NAME examples_c
      WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
      COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=c"
      )
    list(APPEND examples_compare_DEPENDS examples_c)

    if(ENABLE_cxx)
      add_test(NAME examples_cxx
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=cxx"
	)
      list(APPEND examples_compare_DEPENDS examples_cxx)
    endif(ENABLE_cxx)

    if(ENABLE_fortran)
      add_test(NAME examples_fortran
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=fortran"
	)
      list(APPEND examples_compare_DEPENDS examples_fortran)
    endif(ENABLE_fortran)

    if(ENABLE_java)
      if(CORE_BUILD)
	string(REPLACE " " "\\ " PLPLOT_JAVA_WRAP_DIR_ESCAPED ${CMAKE_BINARY_DIR}/bindings/java/)
	string(REPLACE " " "\\ " PLPLOT_CLASSPATH_ESCAPED ${CMAKE_BINARY_DIR}/examples/java/plplot.jar)
	set(JAVA_TEST_ENVIRONMENT "${TEST_ENVIRONMENT} PLPLOT_JAVA_WRAP_DIR=${PLPLOT_JAVA_WRAP_DIR_ESCAPED} PLPLOT_CLASSPATH=${PLPLOT_CLASSPATH_ESCAPED}")
      else(CORE_BUILD)
	string(REPLACE " " "\\ " PLPLOT_JAVA_WRAP_DIR_ESCAPED ${LIB_DIR}/)
	set(JAVA_TEST_ENVIRONMENT "${TEST_ENVIRONMENT} PLPLOT_JAVA_WRAP_DIR=${PLPLOT_JAVA_WRAP_DIR_ESCAPED}")
      endif(CORE_BUILD)

      add_test(NAME examples_java
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${JAVA_TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=java"
	)
      list(APPEND examples_compare_DEPENDS examples_java)
    endif(ENABLE_java)

    if(ENABLE_octave)
      add_test(NAME examples_octave
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=octave"
	)
      list(APPEND examples_compare_DEPENDS examples_octave)
    endif(ENABLE_octave)

    if(ENABLE_python)
      add_test(NAME examples_python
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=python"
	)
      list(APPEND examples_compare_DEPENDS examples_python)
    endif(ENABLE_python)

    if(ENABLE_tcl)
      add_test(NAME examples_tcl
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=tcl"
	)
      list(APPEND examples_compare_DEPENDS examples_tcl)
    endif(ENABLE_tcl)

    if(ENABLE_ada)
      add_test(NAME examples_ada
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=ada"
	)
      list(APPEND examples_compare_DEPENDS examples_ada)
    endif(ENABLE_ada)

    if(ENABLE_ocaml)
      add_test(NAME examples_ocaml
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=ocaml"
	)
      list(APPEND examples_compare_DEPENDS examples_ocaml)
    endif(ENABLE_ocaml)

    if(ENABLE_lua)
      add_test(NAME examples_lua
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=lua"
	)
      list(APPEND examples_compare_DEPENDS examples_lua)
    endif(ENABLE_lua)

    if(ENABLE_d)
      add_test(NAME examples_d
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=d"
	)
      list(APPEND examples_compare_DEPENDS examples_d)
    endif(ENABLE_d)

    if(PLD_plmeta)
      add_test(NAME examples_plrender
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT_DEVICE} --front-end=plrender"
	)
      list(APPEND examples_compare_DEPENDS examples_plrender)
    endif(PLD_plmeta)

  endif(PLD_TEST_DEVICE)

  # FILE_DEVICES_LIST is set in cmake/modules/drivers-finish.cmake
  # for all enabled _file_ devices and adjusted above for the
  # special case of the psc and psttfc devices.
  # Each list member consists of a colon-separated device name, driver
  # name, and a Boolean variable that indicates if the device has familied
  # output in plplot-test.sh (i.e., requires familying).  Only the
  # enabled device name is important in this context.

  foreach(file_devices_info ${FILE_DEVICES_LIST})
    string(REGEX REPLACE "^(.*):.*:.*$" "\\1" device ${file_devices_info})
    if (NOT PLPLOT_TEST_DEVICE STREQUAL device)
      add_test(NAME examples_${device}
        WORKING_DIRECTORY ${TEST_SCRIPT_DIRECTORY}
	COMMAND ${SH_EXECUTABLE} -c "${TEST_ENVIRONMENT} ${TEST_SCRIPT} --front-end=c --device=${device}"
	)
    endif(NOT PLPLOT_TEST_DEVICE STREQUAL device)
  endforeach(file_devices_info ${FILE_DEVICES_LIST})

  if(TEST_DIFF)
    string(REPLACE " " "\\ " TEST_DIFF_SH "${TEST_SCRIPT_DIRECTORY}/test_diff.sh")
    add_test(NAME examples_compare
      WORKING_DIRECTORY ${CTEST_EXAMPLES_OUTPUT_DIR}
      COMMAND ${SH_EXECUTABLE} -c "${TEST_DIFF_SH} --device=${PLPLOT_TEST_DEVICE} --familied_device=${FAMILIED_PLPLOT_TEST_DEVICE}"
      )
    # Always force examples_compare to be run after the other tests it depends on.
    #message(STATUS "DEBUG: examples_compare_DEPENDS = ${examples_compare_DEPENDS}")
    set_tests_properties(examples_compare
      PROPERTIES
      DEPENDS "${examples_compare_DEPENDS}"
      )
  endif(TEST_DIFF)

  if(BUILD_TEST)
    if(CORE_BUILD)
      # Make a copy of Chloe.pgm to the test subdirectory of the build
      # tree so that example 20 will run from there.
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Chloe.pgm
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_SOURCE_DIR}/examples/Chloe.pgm
	${CMAKE_CURRENT_BINARY_DIR}/Chloe.pgm
	DEPENDS ${CMAKE_SOURCE_DIR}/examples/Chloe.pgm
	)
      add_custom_target(Chloe_file ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/Chloe.pgm)
    endif(CORE_BUILD)

    # Implement custom target to remove all examples output files
    # created by ctest in a convenient way. Use special directory to
    # store these files since otherwise an attempt to remove these
    # files by name from a multi-use directory can exceed the maximum
    # command-line length for the "MinGW Makefiles" cmd environment.
    add_custom_target(clean_ctest_plot_files
      COMMAND ${CMAKE_COMMAND} -E remove_directory ${CTEST_EXAMPLES_OUTPUT_DIR}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${CTEST_EXAMPLES_OUTPUT_DIR}
      )

  endif(BUILD_TEST)

endif(SH_EXECUTABLE)
