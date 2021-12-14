# examples/CMakeLists.txt

# Copyright (C) 2006-2015 Andrew Ross
# Copyright (C) 2006-2019 Alan W. Irwin
# Copyright (C) 2008-2009 Werner Smekal
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
# and optionally [depending on BUILD_TEST] builds them) and the installed
# examples build.  The core build has BUILD_TEST OFF or ON at user option
# and CORE_BUILD always ON.  The installed examples build always has
# BUILD_TEST ON and CORE_BUILD OFF.

if(CORE_BUILD)
  # Configure Makefile.examples with some specific variables for the
  # traditional Makefile + pkg-config installed examples build.

  # cxx and tk subdirectories are special cases.
  if(PLD_${PLPLOT_TEST_DEVICE})
    # DEVICE_C_COMMENT not disabled so must disable UPPERCASED(${PLPLOT_TEST_DEVICE})_COMMENT
    # instead which otherwise would configure the same target.
    string(TOUPPER ${PLPLOT_TEST_DEVICE} UPPERCASED_PLPLOT_TEST_DEVICE)
    set(${UPPERCASED_PLPLOT_TEST_DEVICE}_COMMENT "#")
    if(NOT ENABLE_ada)
      set(DEVICE_ADA_COMMENT "#")
    endif(NOT ENABLE_ada)

    if(NOT ENABLE_cxx)
      set(DEVICE_CXX_COMMENT "#")
    endif(NOT ENABLE_cxx)

    if(NOT ENABLE_d)
      set(DEVICE_D_COMMENT "#")
    endif(NOT ENABLE_d)

    if(NOT ENABLE_fortran)
      set(DEVICE_FORTRAN_COMMENT "#")
    endif(NOT ENABLE_fortran)

    if(NOT ENABLE_java)
      set(DEVICE_JAVA_COMMENT "#")
    endif(NOT ENABLE_java)

    if(NOT ENABLE_ocaml)
      set(DEVICE_OCAML_COMMENT "#")
    endif(NOT ENABLE_ocaml)

    if(NOT ENABLE_octave)
      set(DEVICE_OCTAVE_COMMENT "#")
    endif(NOT ENABLE_octave)

    if(NOT ENABLE_python)
      set(DEVICE_PYTHON_COMMENT "#")
    endif(NOT ENABLE_python)

    if(NOT ENABLE_tcl)
      set(DEVICE_TCL_COMMENT "#")
    endif(NOT ENABLE_tcl)

    if(NOT ENABLE_lua)
      set(DEVICE_LUA_COMMENT "#")
    endif(NOT ENABLE_lua)

  else(PLD_${PLPLOT_TEST_DEVICE})
    set(DEVICE_ADA_COMMENT "#")
    set(DEVICE_C_COMMENT "#")
    set(DEVICE_CXX_COMMENT "#")
    set(DEVICE_D_COMMENT "#")
    set(DEVICE_FORTRAN_COMMENT "#")
    set(DEVICE_JAVA_COMMENT "#")
    set(DEVICE_OCAML_COMMENT "#")
    set(DEVICE_OCTAVE_COMMENT "#")
    set(DEVICE_PYTHON_COMMENT "#")
    set(DEVICE_TCL_COMMENT "#")
    set(DEVICE_LUA_COMMENT "#")
  endif(PLD_${PLPLOT_TEST_DEVICE})

  if(NOT(ENABLE_cxx AND (PLD_${PLPLOT_TEST_DEVICE} OR ENABLE_qt OR ENABLE_wxwidgets)))
    set(CXX_COMMENT "#")
  endif(NOT(ENABLE_cxx AND (PLD_${PLPLOT_TEST_DEVICE} OR ENABLE_qt OR ENABLE_wxwidgets)))

  if(NOT ENABLE_tkX)
    set(TK_COMMENT "#")
  endif(NOT ENABLE_tkX)

  if(NOT PLD_pdfcairo)
    set(PDFCAIRO_COMMENT "#")
  endif(NOT PLD_pdfcairo)

  if(NOT PLD_pngcairo)
    set(PNGCAIRO_COMMENT "#")
  endif(NOT PLD_pngcairo)

  if(NOT PLD_pscairo)
    set(PSCAIRO_COMMENT "#")
  endif(NOT PLD_pscairo)

  if(NOT PLD_epscairo)
    set(EPSCAIRO_COMMENT "#")
  endif(NOT PLD_epscairo)

  if(NOT PLD_svgcairo)
    set(SVGCAIRO_COMMENT "#")
  endif(NOT PLD_svgcairo)

  if(NOT PLD_gif)
    set(GIF_COMMENT "#")
  endif(NOT PLD_gif)

  if(NOT PLD_jpeg)
    set(JPEG_COMMENT "#")
  endif(NOT PLD_jpeg)

  if(NOT PLD_png)
    set(PNG_COMMENT "#")
  endif(NOT PLD_png)

  if(NOT PLD_ps)
    set(PS_COMMENT "#")
  endif(NOT PLD_ps)

  if(NOT PLD_psc)
    set(PSC_COMMENT "#")
  endif(NOT PLD_psc)

  if(NOT PLD_psttf)
    set(PSTTF_COMMENT "#")
  endif(NOT PLD_psttf)

  if(NOT PLD_psttfc)
    set(PSTTFC_COMMENT "#")
  endif(NOT PLD_psttfc)

  if(NOT PLD_svg)
    set(SVG_COMMENT "#")
  endif(NOT PLD_svg)

  if(NOT PLD_xfig)
    set(XFIG_COMMENT "#")
  endif(NOT PLD_xfig)

  if(NOT PLD_pstex)
    set(PSTEX_COMMENT "#")
  endif(NOT PLD_pstex)

  if(TEST_DIFF)
    set(COMPARE_COMMENT)
  else(TEST_DIFF)
    set(COMPARE_COMMENT "#")
  endif(TEST_DIFF)

  if(NOT PLD_cgm)
    set(CGM_COMMENT "#")
  endif(NOT PLD_cgm)

  if(NOT PLD_bmpqt)
    set(BMPQT_COMMENT "#")
  endif(NOT PLD_bmpqt)

  if(NOT PLD_jpgqt)
    set(JPGQT_COMMENT "#")
  endif(NOT PLD_jpgqt)

  if(NOT PLD_pngqt)
    set(PNGQT_COMMENT "#")
  endif(NOT PLD_pngqt)

  if(NOT PLD_ppmqt)
    set(PPMQT_COMMENT "#")
  endif(NOT PLD_ppmqt)

  if(NOT PLD_tiffqt)
    set(TIFFQT_COMMENT "#")
  endif(NOT PLD_tiffqt)

  if(NOT PLD_svgqt)
    set(SVGQT_COMMENT "#")
  endif(NOT PLD_svgqt)

  if(NOT PLD_epsqt)
    set(EPSQT_COMMENT "#")
  endif(NOT PLD_epsqt)

  if(NOT PLD_pdfqt)
    set(PDFQT_COMMENT "#")
  endif(NOT PLD_pdfqt)

  # Note that the results configured in the Requires,
  # Requires.private, Libs, and Libs.private lines of the PLplot *.pc
  # files used in our traditional build of the installed examples
  # depend on whether NON_TRANSITIVE is ON or OFF.
  if(BUILD_SHARED_LIBS)
    # pkg-config will not use the --static pkg-config option for the
    # traditional installed examples build, i.e., the configured
    # Requires.private and Libs.private results will be completely
    # ignored in the PLplot *.pc files.
    set(PC_STATIC_OPTION)
  else(BUILD_SHARED_LIBS)
    # pkg-config will use the --static pkg-config option for the
    # traditional installed examples build, i.e., the configured
    # Requires.private results will be appended to the
    # Requires results and the configured Libs.private results will
    # be appended to the Libs results.
    set(PC_STATIC_OPTION "--static")
  endif(BUILD_SHARED_LIBS)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    @ONLY
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples
    RENAME Makefile
    )

  if(ENABLE_fortran)
    # According to the principle of "least surprise", the PLPLOT::plfortrandemolib target
    # should be created in the fortran subdirectory.  However, we must process that subdirectory
    # *after* the the foreach loop below where the PLPLOT::plfortrandemolib target is required.
    # So to break this "chicken/egg" issue build the PLPLOT::plfortrandemolib target here.

    # The module files for the fortran binding are created by
    # default during the library build in the bindings/fortran directory.
    include_directories(${CMAKE_BINARY_DIR}/bindings/fortran)
    # plfortrandemolib depends on the plplotfortran library.
    set(LIB_INSTALL_RPATH ${LIB_DIR})
    configure_library_build(plfortrandemolib STATIC fortran/plfortrandemolib.f90 PLPLOT::plplotfortran "${LIB_INSTALL_RPATH}")
    install(FILES
      ${CMAKE_CURRENT_BINARY_DIR}/plfortrandemolib.mod
      DESTINATION ${FORTRAN_MOD_DIR}
      )
  endif(ENABLE_fortran)

  if(USE_RPATH AND NOT WIN32_OR_CYGWIN)
    # Sort out RPATH issues for the traditional build of the installed
    # examples using */Makefiles configured in each * subdirectory
    # from */Makefile.examples.in.  N.B. We specifically exclude
    # WIN32_OR_CYGWIN here for the traditional build of the installed
    # examples to follow CMake's lead where our evidence is the
    # INSTALL_RPATH property (always set by us when USE_RPATH is ON)
    # is ignored (presumably because CMake developers do not feel
    # rpath is [sufficiently well] supported on those platforms.
    # Note, WIN32_OR_CYGWIN users normally deal with what would be
    # rpath issues on POSIX systems by adjusting their PATH to include
    # all library locations.  And normally PLplot packagers on POSIX
    # systems set USE_RPATH to OFF because setting rpath interferes
    # with using LD_LIBRARY_PATH (a concern for some packagers) and
    # installed library locations are system locations that are
    # automatically searched for libraries by the run-time loader in
    # any case.

    # All examples compiled from installed source code depend on libraries installed in ${LIB_DIR}.
    # All additional information about internal library dependencies below taken
    # from the examples/<language>/CMakeLists.txt target_link_libraries
    # commands.

    # Create list of comma-separated dictionary with the index being
    # the language associated with the created
    # install_tree_<language>_RPATH variable, and the dictionary value
    # being the second and subsequent comma-separated list of internal
    # library dependencies.
    set(rpath_process_dictionary_list
      c,PLPLOT::plplot
      )
    if(ENABLE_ada)
      list(APPEND rpath_process_dictionary_list ada,PLPLOT::plplotada,PLPLOT::plplot)
    endif(ENABLE_ada)

    if(ENABLE_cxx)
      list(APPEND rpath_process_dictionary_list cxx,PLPLOT::plplotcxx)
    endif(ENABLE_cxx)

    # wxwidgets example:
    if(ENABLE_wxwidgets)
      list(APPEND rpath_process_dictionary_list wxwidgets,PLPLOT::plplotwxwidgets)
    endif(ENABLE_wxwidgets)

    # qt example:
    if(PLD_extqt)
      if(ENABLE_DYNDRIVERS)
	list(APPEND rpath_process_dictionary_list qt,PLPLOT::plplotqt,PLPLOT::plplot)
      else(ENABLE_DYNDRIVERS)
	list(APPEND rpath_process_dictionary_list qt,PLPLOT::plplot)
      endif(ENABLE_DYNDRIVERS)
    endif(PLD_extqt)

    # d examples:
    if(ENABLE_d)
      list(APPEND rpath_process_dictionary_list d,PLPLOT::plplotdmd,PLPLOT::plplot)
    endif(ENABLE_d)

    # fortran examples:
    if(ENABLE_fortran)
      list(APPEND rpath_process_dictionary_list fortran,PLPLOT::plfortrandemolib)
    endif(ENABLE_fortran)

    # tk examples:
    if(ENABLE_tk)
      if(tcltk_in_plplot_library)
	list(APPEND rpath_process_dictionary_list tk,PLPLOT::plplottcltk_Main,PLPLOT::plplot)
      else(tcltk_in_plplot_library)
	list(APPEND rpath_process_dictionary_list tk,PLPLOT::plplottcltk_Main,PLPLOT::plplottcltk,PLPLOT::tclmatrix,PLPLOT::plplot)
      endif(tcltk_in_plplot_library)
    endif(ENABLE_tk)

    foreach(rpath_process_dictionary ${rpath_process_dictionary_list})
      # Parse comma-separated dictionary index and second and subsequent comma-separated list
      string(REGEX REPLACE "^([^,]*),.*$" "\\1" language ${rpath_process_dictionary})
      string(REGEX REPLACE "^[^,]*,(.*)$" "\\1" tll_arguments ${rpath_process_dictionary})
      #message(STATUS "DEBUG: language = ${language}")
      #message(STATUS "DEBUG: tll_arguments = ${tll_arguments}")
      # Each of the installed examples that are built depend on a library installed in ${LIB_DIR}.
      set(install_tree_${language}_RPATH ${LIB_DIR})
      # Transform tll_arguments from a comma- to a semi-colon-separated list (i.e., an official CMake list).
      string(REGEX REPLACE "," ";" tll_arguments ${tll_arguments})
      process_rpath(install_tree_${language}_RPATH "${tll_arguments}")
      # Transform from semicolon- to colon-separated list.  (Quotes
      # for last argument required to process whole argument as string containing
      # semicolons rather than as the concatanated elements of the list (without semicolons).
      string(REGEX REPLACE ";" ":"  install_tree_${language}_RPATH "${install_tree_${language}_RPATH}")
      set(install_tree_${language}_RPATHCMD "-Wl,-rpath -Wl,\"${install_tree_${language}_RPATH}\"")
      #message(STATUS "DEBUG: install_tree_${language}_RPATHCMD = ${install_tree_${language}_RPATHCMD}")
    endforeach(rpath_process_dictionary ${rpath_process_dictionary_list})

    # nagfor compiler is a special case:
    if(install_tree_fortran_RPATHCMD AND CMAKE_Fortran_COMPILER MATCHES "nagfor")
      # Extra layer of -Wl indirection required for the nagfor compiler
      string(REPLACE "-Wl," "-Wl,-Wl,," install_tree_fortran_RPATHCMD ${install_tree_fortran_RPATHCMD})
    endif(install_tree_fortran_RPATHCMD AND CMAKE_Fortran_COMPILER MATCHES "nagfor")

    # ocaml is a special case because the (unofficial) CMake language support is
    # implemented with low-level add_custom commands/targets so that rpath
    # must be specified for 3 cases (the traditional build of the installed
    # examples, and the CMake-based build of the build-tree and install-tree examples).
    # Furthermore, the rpath syntax is different for ocaml.
    if(ENABLE_ocaml)
      set(language ocaml)
      # Each of the installed examples that are built depend on a library installed in ${LIB_DIR}.
      set(install_tree_${language}_RPATH ${LIB_DIR})
      # The ocaml examples depend on the plplot library
      set(tll_arguments PLPLOT::plplot)
      process_rpath(install_tree_${language}_RPATH "${tll_arguments}")

      # Special case of the ocaml build-tree examples.
      # Must be done before ${install_tree_${language}_RPATH gets transformed
      # from official CMake list to colon-separated form.
      set(build_tree_${language}_RPATH ${install_tree_${language}_RPATH})
      list(REMOVE_ITEM build_tree_${language}_RPATH ${LIB_DIR})
      list(APPEND build_tree_${language}_RPATH ${CMAKE_BINARY_DIR}/src)

      # Transform from semicolon- to colon-separated list.  (Quotes
      # for last argument required to process whole argument as string
      # containing semicolons rather than as the concatanated elements
      # of the list (without semicolons).
      # N.B. Mixture of unescaped and escaped forms needed for both
      # install- and build-tree versions of CMake-based build of ocaml
      # examples
      string(REGEX REPLACE ";" ":"  install_tree_${language}_RPATH "${install_tree_${language}_RPATH}")
      string(REGEX REPLACE ";" ":"  build_tree_${language}_RPATH "${build_tree_${language}_RPATH}")
      string(REPLACE " " "\\ " install_tree_${language}_RPATH_ESCAPED "${install_tree_${language}_RPATH}")
      string(REPLACE " " "\\ " build_tree_${language}_RPATH_ESCAPED "${build_tree_${language}_RPATH}")

      # install_tree_ocaml_RPATHCMD needed in configured traditional
      # Makefile for the ocaml subdirectory.
      # Ocaml compiler requires -ccopt in front of every -Wl option:
      set(install_tree_${language}_RPATHCMD "-ccopt -Wl,-rpath -ccopt -Wl,\"${install_tree_${language}_RPATH_ESCAPED}\"")

    endif(ENABLE_ocaml)


  endif(USE_RPATH AND NOT WIN32_OR_CYGWIN)

  # Install Chloe.pgm in installed examples directory so that all implementations
  # of example 20 in the various examples subdirectories can conveniently
  # access this file.
  install(
    FILES
    ${CMAKE_CURRENT_SOURCE_DIR}/Chloe.pgm
    ${CMAKE_CURRENT_SOURCE_DIR}/README.Chloe
    DESTINATION ${DATA_DIR}/examples
    )

endif(CORE_BUILD)

if(CORE_BUILD)
  # These variables (normally not set or needed for a core build) are
  # needed below for the tests of the languages and devices.  They
  # are also needed to configure plplot_configure.cmake whose template
  # file is plplot_configure.cmake_installed_examples.in and which
  # ends up installed in examples/cmake/module.
  set(ENABLE_c ON)

  # Start configuration/installation of CMake-based build system for
  # installed examples.
  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples
    )

  # language_info_LIST contains information about the languages used in
  # the noninteractive tests.  The first field is language (which must be
  # consistent with the list of languages configured by set commands in
  # examples/plplot_configure.cmake_installed_examples.in).  The second
  # field is the subdirectory corresponding to the language.  The third
  # field is the file suffix on plplot-test.sh results for that language.
  # N.B. this list is configured in plplot_configure.cmake below and
  # is therefore available both for the core build of the tests and
  # the installed-examples build of the tests below.

  set(language_info_LIST
    ada:ada:a
    c:c:c
    cxx:c++:cxx
    d:d:d
    fortran:fortran:f
    java:java:j
    lua:lua:lua
    ocaml:ocaml:ocaml
    octave:octave:o
    python:python:p
    tcl:tcl:t
    plrender:c:plm
    )

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plplot_configure.cmake_installed_examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_configure.cmake_installed_examples
    @ONLY
    )

  install(FILES
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_configure.cmake_installed_examples
    DESTINATION ${DATA_DIR}/examples/cmake/modules
    RENAME plplot_configure.cmake
    )

  # function support for CMake-based build system for installed examples.
  install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/modules/plplot_functions.cmake
    DESTINATION ${DATA_DIR}/examples/cmake/modules
    )

  # pkg-config support for CMake-based build system for installed examples.
  install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/modules/pkg-config.cmake
    DESTINATION ${DATA_DIR}/examples/cmake/modules
    )

  # Install soft-landing support for compiler detection/testing.
  install(FILES
    ${CMAKE_SOURCE_DIR}/cmake/modules/language_support.cmake
    DESTINATION ${DATA_DIR}/examples/cmake/modules
    )

  # Install qt support file.
  if(ANY_QT_DEVICE)
    install(FILES
      ${CMAKE_SOURCE_DIR}/cmake/modules/ndp_UseQt4.cmake
      DESTINATION ${DATA_DIR}/examples/cmake/modules
      )
  endif(ANY_QT_DEVICE)

  # Install PLplot-specific language support files needed for CMake-based
  # build of installed examples.
  # N.B. This list of files must be maintained consistently with the file
  # lists in cmake/modules/language_support.cmake.

  set(LANG_SUPPORT_FILES)

  if(ENABLE_ada)
    set(LANG_SUPPORT_FILES ${LANG_SUPPORT_FILES}
      language_support/cmake:CMakeAdaCompiler.cmake.in
      language_support/cmake:CMakeAdaInformation.cmake
      language_support/cmake:CMakeDetermineAdaCompiler.cmake
      language_support/cmake:CMakeTestAdaCompiler.cmake
      language_support/cmake/Platform:CYGWIN-GNU-Ada.cmake
      language_support/cmake/Platform:Darwin-GNU-Ada.cmake
      language_support/cmake/Platform:Linux-GNU-Ada.cmake
      language_support/cmake/Platform:Windows-GNU-Ada.cmake
      language_support/cmake/Compiler:GNU-Ada.cmake
      )
  endif(ENABLE_ada)

  if(ENABLE_d)
    set(LANG_SUPPORT_FILES ${LANG_SUPPORT_FILES}
      language_support/cmake:CMakeD_Copyright.txt
      language_support/cmake:CMakeDCompiler.cmake.in
      language_support/cmake:CMakeDInformation.cmake
      language_support/cmake:CMakeDetermineDCompiler.cmake
      language_support/cmake:CMakeTestDCompiler.cmake
      language_support/cmake/Platform:Darwin-dmd.cmake
      language_support/cmake/Platform:Linux-dmd.cmake
      language_support/cmake/Platform:Linux-gdc.cmake
      language_support/cmake/Platform:Windows-dmd.cmake
      language_support/cmake/Platform:Windows-gdc.cmake
      language_support/cmake/Platform:Windows-gdc.cmake
      )
  endif(ENABLE_d)

  if(LANG_SUPPORT_FILES)
    foreach(LANG_SUPPORT_info ${LANG_SUPPORT_FILES})
      string(REGEX REPLACE "^(.*):.*$" "\\1" LANG_SUPPORT_DIR ${LANG_SUPPORT_info})
      string(REGEX REPLACE "^.*:(.*)$" "\\1" LANG_SUPPORT_FILE ${LANG_SUPPORT_info})
      install(FILES
	${CMAKE_SOURCE_DIR}/cmake/modules/${LANG_SUPPORT_DIR}/${LANG_SUPPORT_FILE}
	DESTINATION ${DATA_DIR}/examples/cmake/modules/${LANG_SUPPORT_DIR}
	)
    endforeach(LANG_SUPPORT_info ${LANG_SUPPORT_FILES})
  endif(LANG_SUPPORT_FILES)

else(CORE_BUILD)

  # MAINTENANCE 2019-02-03
  # This logic copied exactly (including version numbers which change
  # from time to time) from the top-level CMakeLists.txt file except
  # for the name of the project.
  set(MINIMUM_LINUX_CMAKE_VERSION 3.13.2)
  # Latest CMake version for all platforms other than Linux.
  # Must be greater than or equal to MINIMUM_LINUX_CMAKE_VERSION
  set(MINIMUM_NON_LINUX_CMAKE_VERSION 3.13.2)
  # Keep policy consistent for all platforms, i.e., use the Linux
  # version which must be the minimum of the two versions above.
  set(CMAKE_UNIFORM_POLICY_VERSION ${MINIMUM_LINUX_CMAKE_VERSION})
  cmake_minimum_required(VERSION ${MINIMUM_LINUX_CMAKE_VERSION} FATAL_ERROR)
  cmake_policy(VERSION ${CMAKE_UNIFORM_POLICY_VERSION})
  project(installed_plplot_examples NONE)

  if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    cmake_minimum_required(VERSION ${MINIMUM_NON_LINUX_CMAKE_VERSION} FATAL_ERROR)
    cmake_policy(VERSION ${CMAKE_UNIFORM_POLICY_VERSION})
  endif(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")

  # CMP0086 implemented for cmake-3.14.0 so to avoid warning
  # messages when our minimum version is less than that must
  # set this policy when it is available.
  if(POLICY CMP0086)
    cmake_policy(SET CMP0086 NEW)
  endif(POLICY CMP0086)

  # It is a fatal error if no working C compiler is available to build
  # the PLplot core C library and core C examples.  All other compilers
  # required by our bindings are optional in that if no working compiler
  # of the kind needed is available, the associated bindings and
  # examples are disabled.
  enable_language(C)

  message(STATUS "CMAKE_SYSTEM_NAME = ${CMAKE_SYSTEM_NAME}")
  message(STATUS "CMAKE_COMMAND = ${CMAKE_COMMAND}")
  message(STATUS "CMAKE_VERSION = ${CMAKE_VERSION}")
  message(STATUS "CMAKE_GENERATOR = ${CMAKE_GENERATOR}")
  message(STATUS "CMAKE_INCLUDE_PATH = ${CMAKE_INCLUDE_PATH}")
  message(STATUS "Environment variable CMAKE_INCLUDE_PATH = $ENV{CMAKE_INCLUDE_PATH}")
  message(STATUS "CMAKE_LIBRARY_PATH = ${CMAKE_LIBRARY_PATH}")
  message(STATUS "Environment variable CMAKE_LIBRARY_PATH = $ENV{CMAKE_LIBRARY_PATH}")
  # Location of configured language support files.
  message(STATUS "CMAKE_PLATFORM_INFO_DIR = ${CMAKE_PLATFORM_INFO_DIR}")

  # Locations where the PLplot build system first looks for cmake modules.
  set(CMAKE_MODULE_PATH
    "${PROJECT_SOURCE_DIR}/cmake/modules"
    "${PROJECT_SOURCE_DIR}/cmake/modules/language_support/cmake"
    )

  #Configure variables and enable languages as needed.
  include(plplot_configure)

  # Maintenance: 2019-03-29.  Copy all the ctest stuff from the top-level directory
  # Enable CTest-based testing framework for examples.
  if(BUILD_TEST)

    # Use same BUILDNAME logic as the CTest module except
    # we allow a BUILDNAME suffix whose value is specified
    # by the user as PLPLOT_BUILDNAME_SUFFIX.  In order
    # for this logic to work it must precede including the
    # CTest module which configures DartConfiguration.tcl
    # with BUILDNAME as determined here.
    # Maintenance: 2019-03-28 copy this BUILDNAME logic from
    # cmake git v3.13.4 Modules/CTest.cmake.
    if(NOT BUILDNAME)
      set(DART_COMPILER "${CMAKE_CXX_COMPILER}")
      if(NOT DART_COMPILER)
	set(DART_COMPILER "${CMAKE_C_COMPILER}")
      endif()
      if(NOT DART_COMPILER)
	set(DART_COMPILER "unknown")
      endif()
      if(WIN32)
	set(DART_NAME_COMPONENT "NAME_WE")
      else()
	set(DART_NAME_COMPONENT "NAME")
      endif()
      if(NOT BUILD_NAME_SYSTEM_NAME)
	set(BUILD_NAME_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}")
      endif()
      if(WIN32)
	set(BUILD_NAME_SYSTEM_NAME "Win32")
      endif()
      if(UNIX OR BORLAND)
	get_filename_component(DART_COMPILER_NAME
          "${DART_COMPILER}" ${DART_NAME_COMPONENT})
      else()
	get_filename_component(DART_COMPILER_NAME
          "${CMAKE_MAKE_PROGRAM}" ${DART_NAME_COMPONENT})
      endif()
      if(DART_COMPILER_NAME MATCHES "devenv")
	GET_VS_VERSION_STRING("${CMAKE_GENERATOR}" DART_COMPILER_NAME)
      endif()
      set(BUILDNAME "${BUILD_NAME_SYSTEM_NAME}-${DART_COMPILER_NAME}")
    endif()

    # This is only BUILDNAME logic that is different from the CTest.cmake version.
    set(BUILDNAME "${BUILDNAME}${PLPLOT_BUILDNAME_SUFFIX}")

    # Change the default ctest timeout from 1500 to 15000 to accommodate
    # our users who happen to have extraordinarily slow computers (such
    # as the Raspberry Pi) or our users with anomalous platform issues
    # where some tests are extraordinarily slow (such as test_c_epsqt
    # and test_c_pdfqt for the static library build on MinGW-w64/MSYS2
    # [as of 2017-08]).

    # (This change must also be done before the following include)
    set(DART_TESTING_TIMEOUT 15000 CACHE STRING
      "Maximum time allowed before CTest will kill the test.")

    # According to
    # <https://cmake.org/Wiki/CMake_Testing_With_CTest>
    # the following command executes the required "enable_testing()"
    # command and also enable the creation and submission of a dashboard
    # (which is defined as the result of a test run, reformatted for easy review)
    # to our dashboard server (the PLplot_git "project" at my.cdash.org whose
    # details are given by CTestConfig.cmake and whose dashboard viewer
    # URL is <http://my.cdash.org/index.php?project=PLplot_git>).
    include(CTest)

    # Customize default values set by include(CTest).  Result must be in
    # top of build tree so copy it there.  Also note comment
    # from David Cole <https://blog.kitware.com/ctest-performance-tip-use-ctestcustom-cmake-not-ctest/>
    # that for greatest speed (to avoid a huge glob over the build tree) it should be named
    # CTestCustom.cmake rather than the traditional name CTestCustom.ctest
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/CTestCustom.cmake.in
      ${CMAKE_CURRENT_BINARY_DIR}/CTestCustom.cmake
      COPYONLY
      )
    # N.B. we execute add_test(...) in the plplot_test
    # subdirectory to implement the various tests that are
    # run by ctest.
  endif(BUILD_TEST)

  find_package(plplot)

  add_subdirectory(plplot_test)

endif(CORE_BUILD)

if(CORE_BUILD AND BUILD_TEST)
  remove_definitions("-DPLPLOT_HAVE_CONFIG_H")
endif(CORE_BUILD AND BUILD_TEST)

# Decide on device to be used for generic interactive tests.

if(PLD_xwin)
  # Typically for Unix with X
  set(generic_interactive_device xwin)
elseif(PLD_ntk)
  # Typically for all platforms if ntk is available but not xwin
  set(generic_interactive_device ntk)
elseif(PLD_wingcc)
  # Typically for Windows if -dev ntk is not available
  set(generic_interactive_device wingcc)
else(PLD_xwin)
  set(generic_interactive_device)
endif(PLD_xwin)

# language_info_LIST excludes the tk subdirectly since noninteractive
# tests and file results do not involve tk.  However, the tk
# subdirectory does have to be processed just like the rest
# so add it to this loop.
foreach(language_info ${language_info_LIST} "tk:tk:nothing")
  string(REGEX REPLACE "^(.*):.*:.*$" "\\1" language ${language_info})
  string(REGEX REPLACE "^.*:(.*):.*$" "\\1" subdir ${language_info})
  #message(STATUS "DEBUG: language, subdir = ${language},${subdir}")
  if(ENABLE_${language})
    if(NOT ${language} STREQUAL "plrender")
      add_subdirectory(${subdir})
    endif(NOT ${language} STREQUAL "plrender")
    get_property(targets_examples_${language} GLOBAL PROPERTY TARGETS_examples_${language})
    get_property(files_examples_${language} GLOBAL PROPERTY FILES_examples_${language})
  endif(ENABLE_${language})
endforeach(language_info ${language_info_LIST} "tk:tk:nothing")

# Set up TARGET_tclIndex_tcl and TARGET_tclIndex_tk to account for
# the non-core build where the required files are installed and
# these actual targets do not exist.
if(TARGET tclIndex_tcl)
  set(TARGET_tclIndex_tcl tclIndex_tcl)
else(TARGET tclIndex_tcl)
  set(TARGET_tclIndex_tcl)
endif(TARGET tclIndex_tcl)
if(TARGET tclIndex_tk)
  set(TARGET_tclIndex_tk tclIndex_tk)
else(TARGET tclIndex_tk)
  set(TARGET_tclIndex_tk)
endif(TARGET tclIndex_tk)

if(BUILD_TEST)
  # Chloe.pgm needed in build tree for the examples that are run from there.
  # Done at cmake time without dependencies because cannot see how this
  # file will ever change on short time scales.
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_SOURCE_DIR}/Chloe.pgm
    ${CMAKE_CURRENT_BINARY_DIR}/Chloe.pgm
    )

  find_program(VALGRIND_EXECUTABLE valgrind)
  if(VALGRIND_EXECUTABLE)
    option(VALGRIND_ALL_TESTS "Apply valgrind to all runs of plplot_test/plplot-test.sh" OFF)
    if(VALGRIND_ALL_TESTS)
      set(plplot_test_debug --debug)
    else(VALGRIND_ALL_TESTS)
      set(plplot_test_debug)
    endif(VALGRIND_ALL_TESTS)
  else(VALGRIND_EXECUTABLE)
    set(plplot_test_debug)
  endif(VALGRIND_EXECUTABLE)

  set(TEST_EXAMPLES_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/test_examples_output_dir)
  file(MAKE_DIRECTORY ${TEST_EXAMPLES_OUTPUT_DIR})
  if(CORE_BUILD)
    set(custom_test_script ${CMAKE_BINARY_DIR}/plplot_test/plplot-test.sh)
    set(custom_env EXAMPLES_PREFIX=${CMAKE_BINARY_DIR}/examples SRC_EXAMPLES_PREFIX=${CMAKE_SOURCE_DIR}/examples OUTPUT_DIR=${TEST_EXAMPLES_OUTPUT_DIR})
    set(java_custom_env ${custom_env} PLPLOT_JAVA_WRAP_DIR=${CMAKE_BINARY_DIR}/bindings/java/ PLPLOT_CLASSPATH=${CMAKE_BINARY_DIR}/examples/java/plplot.jar)
    set(compare_script ${CMAKE_BINARY_DIR}/plplot_test/test_diff.sh)
  else(CORE_BUILD)
    set(custom_test_script ${CMAKE_CURRENT_SOURCE_DIR}/plplot-test.sh)
    set(custom_env EXAMPLES_PREFIX=${CMAKE_CURRENT_BINARY_DIR} SRC_EXAMPLES_PREFIX=${CMAKE_CURRENT_SOURCE_DIR} OUTPUT_DIR=${TEST_EXAMPLES_OUTPUT_DIR})
    set(java_custom_env ${custom_env} PLPLOT_JAVA_WRAP_DIR=${LIB_DIR}/)
    set(compare_script ${CMAKE_CURRENT_SOURCE_DIR}/test_diff.sh)
  endif(CORE_BUILD)

  set(custom_test_command ${SH_EXECUTABLE} ${custom_test_script})
  set(compare_command ${SH_EXECUTABLE} ${compare_script})

  set(tcl_targets_LIST)
  if(PLD_${PLPLOT_TEST_DEVICE})
    set(compare_file_depends ${custom_test_script})
    if(ENABLE_DYNDRIVERS)
      get_property(${PLPLOT_TEST_DEVICE}_file_depends GLOBAL PROPERTY FILE_DEPENDS_${PLPLOT_TEST_DEVICE}_dyndriver)
      # ${PLPLOT_TEST_DEVICE}_file_depends contains the filename associated with testing ${PLPLOT_TEST_DEVICE}.
      # It is empty if not a core build.
      # The file dependency on ${PLPLOT_TEST_TARGET} takes care of all
      # target and file dependencies associated with the driver or plplot library target corresponding to ${PLPLOT_TEST_DEVICE}.
      list(APPEND compare_file_depends ${${PLPLOT_TEST_DEVICE}_file_depends} ${PLPLOT_TEST_TARGET})
    endif(ENABLE_DYNDRIVERS)

    set(diff_targets_LIST)
    set(diff_files_LIST)
    # language_info_LIST set above. Each list member consists of a
    # colon-separated language name, subdirectory name,
    # and the associated filename suffix used by plplot-test.sh.
    foreach(language_info ${language_info_LIST})
      string(REGEX REPLACE "^(.*):.*:.*$" "\\1" language ${language_info})
      string(REGEX REPLACE "^.*:.*:(.*)$" "\\1" suffix ${language_info})

      if(ENABLE_${language})
	set(compare_file_depends_${language} ${compare_file_depends})
        #message(STATUS "DEBUG: files_examples_${language} = ${file_examples_${language}}")
        #message(STATUS "DEBUG: targets_examples_${language} = ${targets_examples_${language}}")
	if(files_examples_${language})
	  # files_examples_${language} is true if the
	  # subdirectory created targets with add_custom_target which
	  # depended on an add_custom_command.  For this special case,
	  # you cannot use the created targets for file dependencies
	  # because of CMake implementation constraints.  Instead you
	  # must use _files_ (contained in files_examples_${language}
	  # for file dependencies.
	  # Another case covered here is when the subdirectory
          # contains non-generated examples files.  In this case also you must
	  # (of course) use _files_ for file dependencies.
	  list(APPEND compare_file_depends_${language} ${files_examples_${language}})
	else(files_examples_${language})
	  # files_examples_${language} is only false if the
	  # subdirectory creates targets with add_executable.  For
	  # this special case, CMake allows use of the created targets
	  # for file dependencies.
	  if(targets_examples_${language})
	    list(APPEND compare_file_depends_${language} ${targets_examples_${language}})
	  endif(targets_examples_${language})
	endif(files_examples_${language})
	#message(STATUS "DEBUG: compare_file_depends_${language} = ${compare_file_depends_${language}}")

	if(language STREQUAL "java")
	  set(environment ${java_custom_env})
	else(language STREQUAL "java")
	  set(environment ${custom_env})
	endif(language STREQUAL "java")

	list_example_files(${TEST_EXAMPLES_OUTPUT_DIR} ${PLPLOT_TEST_DEVICE} ${suffix} output_list)
	add_custom_command(
	  OUTPUT ${output_list}
	  COMMAND ${CMAKE_COMMAND} -E echo "Generate ${language} results for ${PLPLOT_TEST_DEVICE} device"
	  COMMAND env ${environment} ${custom_test_command} --verbose --front-end=${language} --device=${PLPLOT_TEST_DEVICE} ${plplot_test_debug}
	  DEPENDS ${compare_file_depends_${language}}
	  VERBATIM
	  )
	add_custom_target(test_${language}_${PLPLOT_TEST_DEVICE}
	  DEPENDS ${output_list}
	  )
	list(APPEND diff_targets_LIST test_${language}_${PLPLOT_TEST_DEVICE})
	list(APPEND diff_files_LIST ${output_list})

	if(language STREQUAL "tcl")
	  list(APPEND tcl_targets_LIST test_${language}_${PLPLOT_TEST_DEVICE})
	endif(language STREQUAL "tcl")

	# If the subdirectory used an add_custom_target (as indicated
	# by both files_examples_${language} and targets_examples_${language}
	# being true), then for that special case must add a target-level
	# dependency as demanded by the CMake implementation.
	if(files_examples_${language} AND targets_examples_${language})
	  #message(STATUS "DEBUG: targets_examples_${language} = ${targets_examples_${language}}")
	  add_dependencies(test_${language}_${PLPLOT_TEST_DEVICE} ${targets_examples_${language}})
	endif(files_examples_${language} AND targets_examples_${language})
	# ${PLPLOT_TEST_DEVICE}_file_depends is empty if test_${PLPLOT_TEST_DEVICE}_dyndriver target is not defined.
	if(TARGET test_${PLPLOT_TEST_DEVICE}_dyndriver)
	  add_dependencies(test_${language}_${PLPLOT_TEST_DEVICE} test_${PLPLOT_TEST_DEVICE}_dyndriver)
	endif(TARGET test_${PLPLOT_TEST_DEVICE}_dyndriver)
      endif(ENABLE_${language})
    endforeach(language_info ${language_info_LIST})

    if(TEST_DIFF)
      # Note this target has complete file and target dependencies for
      # a comparison of stdout and -dev ${PLPLOT_TEST_DEVICE} results for various languages.
      add_custom_target(test_diff_device COMMAND ${compare_command} --device=${PLPLOT_TEST_DEVICE} --familied_device=${FAMILIED_PLPLOT_TEST_DEVICE}
	DEPENDS ${diff_files_LIST}
	WORKING_DIRECTORY ${TEST_EXAMPLES_OUTPUT_DIR}
	)
      add_dependencies(test_diff_device ${diff_targets_LIST})
      set(noninteractive_targets_LIST test_diff_device)
    else(TEST_DIFF)
      set(noninteractive_targets_LIST)
    endif(TEST_DIFF)
  else(PLD_${PLPLOT_TEST_DEVICE})
    set(noninteractive_targets_LIST)
  endif(PLD_${PLPLOT_TEST_DEVICE})

  # ENABLE_c is always ON by construction, but test it anyway for
  # consistency sake.
  if(ENABLE_c)
    set(device_depends ${custom_test_script})
    # See explanation above about what to do with files_examples_${language} and
    # targets_examples_${language}.
    if(files_examples_c)
      list(APPEND device_depends ${files_examples_c})
    else(files_examples_c)
      if(targets_examples_c)
	list(APPEND device_depends ${targets_examples_c})
      endif(targets_examples_c)
    endif(files_examples_c)

    # FILE_DEVICES_LIST is set in cmake/modules/drivers-finish.cmake
    # for all enabled _file_ devices.  It is thus available for the
    # core build.  It is also configured in plplot_configure.cmake and
    # is thus available for the installed examples tests.  Each list
    # member consists of a colon-separated device name, driver name,
    # and a Boolean variable that indicates if device has familied
    # output in plplot-test.sh.
    foreach(file_devices_info ${FILE_DEVICES_LIST})
      string(REGEX REPLACE "^(.*):.*:.*$" "\\1" device ${file_devices_info})
      string(REGEX REPLACE "^.*:(.*):.*$" "\\1" driver ${file_devices_info})
      string(REGEX REPLACE "^.*:.*:(.*)$" "\\1" familied ${file_devices_info})

      # Must exclude device ${PLPLOT_TEST_DEVICE} from this loop because configuration
      # of the test_c_${PLPLOT_TEST_DEVICE} target has already been done above.
      if(NOT device STREQUAL ${PLPLOT_TEST_DEVICE})
	set(${device}_file_device_depends ${device_depends})
	if(ENABLE_DYNDRIVERS)
	  get_property(${driver}_file_depends
	    GLOBAL PROPERTY FILE_DEPENDS_${driver}_dyndriver
	    )
	  list(APPEND ${device}_file_device_depends
	    ${${driver}_file_depends} PLPLOT::${driver}
	    )
	endif(ENABLE_DYNDRIVERS)
	#message("DEBUG:${device}_file_device_depends = ${${device}_file_device_depends}")
	list_example_files(${TEST_EXAMPLES_OUTPUT_DIR} ${device} c output_list)
	add_custom_command(
	  OUTPUT ${output_list}
	  COMMAND ${CMAKE_COMMAND} -E echo "Generate C results for ${device} file device"
	  COMMAND env ${custom_env} ${custom_test_command} --verbose --front-end=c --device=${device} ${plplot_test_debug}
	  DEPENDS
	  ${${device}_file_device_depends}
	  VERBATIM
	  )
	add_custom_target(test_c_${device}
	  DEPENDS ${output_list}
	  )
	if(TARGET test_${driver}_dyndriver)
	  add_dependencies(test_c_${device} test_${driver}_dyndriver)
	endif(TARGET test_${driver}_dyndriver)

	list(APPEND noninteractive_targets_LIST test_c_${device})
	# Follow what was done above.
	if(files_examples_c AND targets_examples_c)
	  add_dependencies(test_c_${device} ${targets_examples_c})
	endif(files_examples_c AND targets_examples_c)
      endif(NOT device STREQUAL ${PLPLOT_TEST_DEVICE})
    endforeach(file_devices_info ${FILE_DEVICES_LIST})

    set(interactive_targets_LIST)
    set(tk_targets_LIST)
    set(qt_targets_LIST)

    # EXTERNAL_DEVICES_LIST determined here from what external devices
    # are enabled.  The required PLD_* data are available both for the
    # core build and (via the configured plplot_configure.cmake) the
    # installed examples build.  Each list member consists of a
    # colon-separated external device name and driver name.

    set(EXTERNAL_DEVICES_LIST)
    if(PLD_extcairo)
      list(APPEND EXTERNAL_DEVICES_LIST "extcairo:cairo")
    endif(PLD_extcairo)
    if(PLD_extqt)
      list(APPEND EXTERNAL_DEVICES_LIST "extqt:qt")
    endif(PLD_extqt)

    # <driver>_targets determined in the loop below over all enabled
    # external devices and interactive devices are used both for (one)
    # custom command DEPENDS below and for several add_dependencies
    # commands below.

    # N.B. INTERACTIVE_DEVICES_LIST is set in
    # cmake/modules/drivers-finish.cmake for all enabled _interactive_
    # devices and is thus available for the core build.  It is also
    # configured in plplot_configure.cmake and is thus available
    # for the installed examples tests.  Each list member consists of
    # a colon-separated device name and driver name.

    foreach(interactive_devices_info IN LISTS EXTERNAL_DEVICES_LIST INTERACTIVE_DEVICES_LIST)
      string(REGEX REPLACE "^(.*):.*$" "\\1" device ${interactive_devices_info})
      string(REGEX REPLACE "^.*:(.*)$" "\\1" driver ${interactive_devices_info})

      # Only process first instance of a particular driver in the list because multiple
      # devices in the list can map to a single driver.
      if(NOT ${driver}_targets)
	if(ENABLE_DYNDRIVERS)
	  set(${driver}_targets PLPLOT::${driver})
	  if(TARGET test_${driver}_dyndriver)
	    list(APPEND ${driver}_targets test_${driver}_dyndriver)
	  endif(TARGET test_${driver}_dyndriver)
	  if(driver STREQUAL "tk" AND TARGET xwin)
	    list(APPEND ${driver}_targets PLPLOT::xwin)
	    if(TARGET test_xwin_dyndriver)
	      list(APPEND ${driver}_targets test_xwin_dyndriver)
	    endif(TARGET test_xwin_dyndriver)
	  endif(driver STREQUAL "tk" AND TARGET xwin)
	else(ENABLE_DYNDRIVERS)
	  set(${driver}_targets PLPLOT::plplot)
	endif(ENABLE_DYNDRIVERS)

	if(driver STREQUAL "tk")
	  list(APPEND ${driver}_targets ${TARGET_tclIndex_tk} ${TARGET_tclIndex_tcl} PLPLOT::plserver)
	endif(driver STREQUAL "tk")

	if(driver STREQUAL "wxwidgets" AND TARGET PLPLOT::wxPLViewer)
	  # Append PLPLOT::wxPLViewer to the list of wxwidgets targets so that application
	  # will be built when needed.
	  list(APPEND ${driver}_targets PLPLOT::wxPLViewer)
	endif(driver STREQUAL "wxwidgets" AND TARGET PLPLOT::wxPLViewer)

      endif(NOT ${driver}_targets)
    endforeach(interactive_devices_info IN LISTS EXTERNAL_DEVICES_LIST EXTERNAL_DEVICES_LIST)

    # Create some custom interactive test targets using list of interactive devices.
    foreach(interactive_devices_info IN LISTS INTERACTIVE_DEVICES_LIST)
      string(REGEX REPLACE "^(.*):.*$" "\\1" device ${interactive_devices_info})
      string(REGEX REPLACE "^.*:(.*)$" "\\1" driver ${interactive_devices_info})

      add_custom_target(test_c_${device}
	COMMAND ${CMAKE_COMMAND} -E echo "Generate C results for ${device} interactive device"
	COMMAND env ${custom_env} ${custom_test_command} --verbose --interactive --device=${device} ${plplot_test_debug}
	VERBATIM
	)
      add_dependencies(test_c_${device} ${${driver}_targets} ${targets_examples_c})
      if(PLPLOT_USE_QT5 AND device STREQUAL "qtwidget")
#	message(STATUS "WARNING: The test_c_qtwidget target can be run independently
#   but is not reliable (see <https://sourceforge.net/p/plplot/bugs/158/>) when
#   PLPLOT_USE_QT5 is set to ${PLPLOT_USE_QT5}.  So until this issue can be straightened out
#   temporarily exclude this target from being a dependency of other more general interactive
#   test targets")
	list(APPEND interactive_targets_LIST test_c_${device})
	list(APPEND qt_targets_LIST test_c_${device})
      else(PLPLOT_USE_QT5 AND device STREQUAL "qtwidget")
	list(APPEND interactive_targets_LIST test_c_${device})
	if(device STREQUAL "tk" OR device STREQUAL "ntk")
	  list(APPEND tk_targets_LIST test_c_${device})
	elseif(device STREQUAL "qtwidget")
	  list(APPEND qt_targets_LIST test_c_${device})
	endif(device STREQUAL "tk" OR device STREQUAL "ntk")
      endif(PLPLOT_USE_QT5 AND device STREQUAL "qtwidget")

      if(ENABLE_octave)
	get_property(targets_examples_octave GLOBAL PROPERTY TARGETS_examples_octave)
	add_custom_target(test_octave_${device}
	  COMMAND ${CMAKE_COMMAND} -E echo "Generate interactive octave results for ${device} interactive device"
	  COMMAND env ${custom_env} ${custom_test_command} --verbose --interactive_octave --device=${device} ${plplot_test_debug}
	  VERBATIM
	  )
	add_dependencies(test_octave_${device} ${targets_examples_octave} ${${driver}_targets})
	message(STATUS "WARNING: The test_octave_${device} target can be run independently but does not
   work reliably so it is temporarily excluded from being a dependency of other
   more general interactive test targets")
	#list(APPEND interactive_targets_LIST test_octave_${device})
	#if(device STREQUAL "tk" OR device STREQUAL "ntk")
	#  list(APPEND tk_targets_LIST test_octave_${device})
	#endif(device STREQUAL "tk" OR device STREQUAL "ntk")
      endif(ENABLE_octave)
    endforeach(interactive_devices_info IN LISTS INTERACTIVE_DEVICES_LIST)

    if(ENABLE_DYNDRIVERS)

      if(TARGET tkwin)
	# tkwin is an external device rather than an interactive one
	# so tkwin_targets (needed for a special case below) is not
	# defined in the above loop and must be defined here instead.
	set(tkwin_targets PLPLOT::tkwin)
	if(TARGET test_tkwin_dyndriver)
	  list(APPEND tkwin_targets test_tkwin_dyndriver)
	endif(TARGET test_tkwin_dyndriver)
      endif(TARGET tkwin)

      if(TARGET cairo)
	# <driver>_file_depends only needed for custom command DEPENDS.  This is
	# the only one of these interactive devices that has a noninteractive
	# test and associated custom_command so it is the only interactive
	# device where <driver>_file_depends needs to be defined.
	get_property(cairo_file_depends
	  GLOBAL PROPERTY FILE_DEPENDS_cairo_dyndriver
	  )
      endif(TARGET cairo)

      # This special target needed for some cases below.
      if(TARGET plplottcltk)
	set(plplottcltk_targets PLPLOT::plplottcltk)
      endif(TARGET plplottcltk)

    else(ENABLE_DYNDRIVERS)

      # MAINTENANCE: the variables set here must be the same as
      # in the above if block
      set(tkwin_targets PLPLOT::plplot)
      set(cairo_file_depends)
      set(plplottcltk_targets PLPLOT::plplot)
    endif(ENABLE_DYNDRIVERS)

    if(PLD_extcairo)
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/ext-cairo-test.ps
	COMMAND ext-cairo-test -drvopt set_background=1
	DEPENDS
	ext-cairo-test
	${cairo_file_depends}
	${cairo_targets}
	)
      add_custom_target(test_extcairo
	DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/ext-cairo-test.ps
	)
      if(TARGET test_cairo_dyndriver)
	add_dependencies(test_extcairo test_cairo_dyndriver)
      endif(TARGET test_cairo_dyndriver)
      list(APPEND noninteractive_targets_LIST test_extcairo)
    endif(PLD_extcairo)

    add_custom_target(test_noninteractive)
    if(noninteractive_targets_LIST)
      add_dependencies(test_noninteractive ${noninteractive_targets_LIST})
    endif(noninteractive_targets_LIST)

    if(PLD_xcairo AND TARGET extXdrawable_demo)
      add_custom_target(test_extXdrawable
	COMMAND extXdrawable_demo
	)
      add_dependencies(test_extXdrawable extXdrawable_demo ${cairo_targets})
      list(APPEND interactive_targets_LIST test_extXdrawable)
    endif(PLD_xcairo AND TARGET extXdrawable_demo)

  endif(ENABLE_c)

  if(ENABLE_cxx)
    if(ENABLE_wxwidgets)
      add_custom_target(test_wxPLplotDemo
	COMMAND wxPLplotDemo
	)
      add_dependencies(test_wxPLplotDemo wxPLplotDemo ${wxwidgets_targets})
      list(APPEND interactive_targets_LIST test_wxPLplotDemo)
    endif(ENABLE_wxwidgets)

    if(PLD_extqt AND TARGET qt_example)
      add_custom_target(test_qt_example
	COMMAND qt_example
	)
      add_dependencies(test_qt_example qt_example ${qt_targets})
      if(PLPLOT_USE_QT5)
	message(STATUS "WARNING: The test_qt_example target can be run independently
   but is not reliable (see <https://sourceforge.net/p/plplot/bugs/158/>) when
   PLPLOT_USE_QT5 is set to ${PLPLOT_USE_QT5}.  So until this issue can be straightened out
   temporarily exclude this target from being a dependency of other more general interactive
   test targets")
	#list(APPEND qt_targets_LIST test_qt_example)
	#list(APPEND interactive_targets_LIST test_qt_example)
      else(PLPLOT_USE_QT5)
	list(APPEND qt_targets_LIST test_qt_example)
	list(APPEND interactive_targets_LIST test_qt_example)
      endif(PLPLOT_USE_QT5)
    endif(PLD_extqt AND TARGET qt_example)

  endif(ENABLE_cxx)

  if(ENABLE_pyqt4)
    if(CORE_BUILD)
      add_custom_target(test_pyqt4_example
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/python/pyqt4_example.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python
	)
      add_dependencies(test_pyqt4_example
	PLPLOT::plplot_pyqt4
	python_examples
	PLPLOT::plplotc
	${qt_targets}
	)
    else(CORE_BUILD)
      add_custom_target(test_pyqt4_example
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/python/pyqt4_example.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
	)
      add_dependencies(test_pyqt4_example
	PLPLOT::plplot_pyqt4
	PLPLOT::plplotc
	${qt_targets}
	)
    endif(CORE_BUILD)

    list(APPEND interactive_targets_LIST test_pyqt4_example)

  endif(ENABLE_pyqt4)

  if(ENABLE_pyqt5)
    if(CORE_BUILD)
      add_custom_target(test_pyqt5_example
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/python/pyqt5_example.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python
	)
      add_dependencies(test_pyqt5_example
	PLPLOT::plplot_pyqt5
	python_examples
	PLPLOT::plplotc
	${qt_targets}
	)
    else(CORE_BUILD)
      add_custom_target(test_pyqt5_example
	COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/python/pyqt5_example.py
	WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
	)
      add_dependencies(test_pyqt5_example
	PLPLOT::plplot_pyqt5
	PLPLOT::plplotc
	${qt_targets}
	)
    endif(CORE_BUILD)
    message(STATUS "WARNING: The test_pyqt5_example target can be run
   independently but it intermittently hangs (at least for
   Qt-5.11.2) so is temporarily excluded from being a dependency of
   other more general interactive test targets")
    #list(APPEND interactive_targets_LIST test_pyqt5_example)
  endif(ENABLE_pyqt5)

  if(ENABLE_tcl AND PLD_${generic_interactive_device})
    # Some of the custom target commands below need this escaped version
    string(REPLACE " " "\\ " CMAKE_CURRENT_BINARY_DIR_ESCAPED ${CMAKE_CURRENT_BINARY_DIR})

    add_custom_target(test_pltcl_standard_examples
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tcl/pltcl_standard_examples -dev ${generic_interactive_device} -np
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tcl
      VERBATIM
      )
    add_dependencies(test_pltcl_standard_examples
      PLPLOT::pltcl
      tcl_examples
      ${TARGET_tclIndex_tcl}
      ${${generic_interactive_device}_targets}
      )
    list(APPEND interactive_targets_LIST test_pltcl_standard_examples)
    list(APPEND tcl_targets_LIST test_pltcl_standard_examples)

  endif(ENABLE_tcl AND PLD_${generic_interactive_device})

  if(ENABLE_tcl AND BUILD_SHARED_LIBS AND PLD_tk)
    # tests that use "package require Pltcl" only work if
    # BUILD_SHARED_LIBS is true and PLD_tk is true.
    add_custom_target(test_tclsh_standard_examples
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tcl/tclsh_standard_examples
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tcl
      VERBATIM
      )

    if(tcltk_in_plplot_library)
      add_dependencies(test_tclsh_standard_examples
	PLPLOT::plplot
	tcl_examples
	${TARGET_tclIndex_tcl}
	${tk_targets}
	)
    else(tcltk_in_plplot_library)
      add_dependencies(test_tclsh_standard_examples
	${plplottcltk_target}
	tcl_examples
	${TARGET_tclIndex_tcl}
	${tk_targets}
	)
    endif(tcltk_in_plplot_library)

    list(APPEND interactive_targets_LIST test_tclsh_standard_examples)
    list(APPEND tcl_targets_LIST test_tclsh_standard_examples)
  endif(ENABLE_tcl AND BUILD_SHARED_LIBS AND PLD_tk)

  if(ENABLE_tkX AND xwin_targets)
    set(targets_examples_tk)

    add_custom_target(test_tk_01
      COMMAND xtk01 -f ${CMAKE_CURRENT_BINARY_DIR_ESCAPED}/tk/tk01
      VERBATIM
      )
    add_dependencies(test_tk_01
      ${TARGET_tclIndex_tcl}
      ${TARGET_tclIndex_tk}
      ${xwin_targets}
      )
    list(APPEND targets_examples_tk test_tk_01)

    add_custom_target(test_tk_03
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tk/tk03
      VERBATIM
      )
    add_dependencies(test_tk_03
      PLPLOT::plserver
      ${TARGET_tclIndex_tcl}
      ${TARGET_tclIndex_tk}
      ${xwin_targets}
      )
    list(APPEND targets_examples_tk test_tk_03)

    add_custom_target(test_tk_plgrid
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tk/plgrid
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tcl
      VERBATIM
      )

    add_dependencies(test_tk_plgrid
      PLPLOT::plserver
      tcl_examples
      ${TARGET_tclIndex_tcl}
      ${TARGET_tclIndex_tk}
      ${xwin_targets}
      )
    list(APPEND targets_examples_tk test_tk_plgrid)

    add_custom_target(test_plserver_standard_examples
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tk/plserver_standard_examples
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tk
      VERBATIM
      )
    add_dependencies(test_plserver_standard_examples
      PLPLOT::plserver
      tcl_examples
      ${TARGET_tclIndex_tcl}
      ${TARGET_tclIndex_tk}
      tclIndex_examples_tk
      ${xwin_targets}
      )
    list(APPEND targets_examples_tk test_plserver_standard_examples)

    # AND (ENABLE_DYNDRIVERS OR PLD_tk) Boolean logic required to
    # insure correct code to support the _Pltk_init module is in
    # libplplot if ENABLE_DYNDRIVERS is OFF.
    if(ENABLE_python AND (ENABLE_DYNDRIVERS OR PLD_tk))
      if(CORE_BUILD)
	add_custom_target(test_pytkdemo
	  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_BINARY_DIR}/python/pytkdemo
	  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/python
	  VERBATIM
	  )
	add_dependencies(test_pytkdemo
	  PLPLOT::plplotc
	  PLPLOT::Pltk_init
	  python_examples
	  ${TARGET_tclIndex_tcl}
	  ${xwin_targets}
	  )
      else(CORE_BUILD)
	add_custom_target(test_pytkdemo
	  COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/python/pytkdemo
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python
	  VERBATIM
	  )
	add_dependencies(test_pytkdemo
	  PLPLOT::plplotc
	  PLPLOT::Pltk_init
	  ${TARGET_tclIndex_tcl}
	  ${xwin_targets}
	  )
      endif(CORE_BUILD)
      message(STATUS "WARNING: The test_pytkdemo target can be run independently but does not
   work reliably (i.e., it sometimes hangs on exit) so it is
   temporarily excluded from being a dependency of other more general
   interactive test targets")
      #list(APPEND interactive_targets_LIST test_pytkdemo)
      #list(APPEND tk_targets_LIST test_pytkdemo)
    endif(ENABLE_python AND (ENABLE_DYNDRIVERS OR PLD_tk))

    if(BUILD_SHARED_LIBS AND PLD_tk)
      # tests that use "package require Pltk" only work if
      # BUILD_SHARED_LIBS is true and PLD_tk is true.
      add_custom_target(test_wish_standard_examples
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tk/wish_standard_examples
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tk
	VERBATIM
	)
      add_dependencies(test_wish_standard_examples
	${plplottcltk_target}
	${tk_targets}
	${xwin_targets}
	tcl_examples
	${TARGET_tclIndex_tcl}
	${TARGET_tclIndex_tk}
	tclIndex_examples_tk
	)
      # Temporarily suspend adding this target to the rest of the Tk
      # interactive targets (and therefore also this target is not added
      # to the overall list of targets run by the test_interactive
      # target) because of a segfault generated by this target.  The
      # cause of that segfault is the Tcl exit command is somehow not
      # compatible with how we have implemented the Tcl plframe command
      # that is run by tkdemos.tcl in the wish case, but that is as much
      # as we know at this stage.
      message(STATUS "WARNING: The test_wish_standard_examples target can be run independently but
   it segfaults at the end so it is temporarily excluded from being a dependency
   of other more general interactive test targets")
      # list(APPEND targets_examples_tk test_wish_standard_examples)
    endif(BUILD_SHARED_LIBS AND PLD_tk)

    if(ENABLE_itkX)
      add_custom_target(test_tk_02
	COMMAND xtk02 -f ${CMAKE_CURRENT_BINARY_DIR}/tk/tk02
	VERBATIM
	)
      add_dependencies(test_tk_02
	${TARGET_tclIndex_tcl}
	${TARGET_tclIndex_tk}
	${xwin_targets}
	)

      if(USE_INCRTCL_VERSION_4)
	message(STATUS "WARNING: The test_tk_02 target can be run independently but it currently
   does not work for version 4 of Itcl and friends so it is temporarily excluded
   from being a dependency of other more general interactive test targets")
      else(USE_INCRTCL_VERSION_4)
	list(APPEND targets_examples_tk test_tk_02)
      endif(USE_INCRTCL_VERSION_4)

      add_custom_target(test_tk_04
	COMMAND xtk04 -f ${CMAKE_CURRENT_BINARY_DIR}/tk/tk04
	VERBATIM
	)
      add_dependencies(test_tk_04
	${TARGET_tclIndex_tcl}
	${TARGET_tclIndex_tk}
	${xwin_targets}
	)
      if(USE_INCRTCL_VERSION_4)
	message(STATUS "WARNING: The test_tk_04 target can be run independently but it currently
   does not work for version 4 of Itcl and friends so it is temporarily excluded
   from being a dependency of other more general interactive test targets")
      else(USE_INCRTCL_VERSION_4)
	list(APPEND targets_examples_tk test_tk_04)
      endif(USE_INCRTCL_VERSION_4)
    endif(ENABLE_itkX)

    add_custom_target(test_plserver_runAllDemos
      COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tk/plserver_runAllDemos
      WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tk
      VERBATIM
      )
    add_dependencies(test_plserver_runAllDemos
      PLPLOT::plserver
      tcl_examples
      ${TARGET_tclIndex_tcl}
      ${TARGET_tclIndex_tk}
      tclIndex_examples_tk
      ${xwin_targets}
      )

    message(STATUS "WARNING: The test_plserver_runAllDemos target can be run independently but
   there are several issues with this target so it is temporarily excluded from being a
   dependency of other more general interactive test targets")
    #list(APPEND targets_examples_tk test_plserver_runAllDemos)

    if(BUILD_SHARED_LIBS AND PLD_tkwin)
      # tests that use "package require Plplotter" only work if
      # BUILD_SHARED_LIBS is true and PLD_tkwin is true.
      add_custom_target(test_wish_runAllDemos
	COMMAND ${CMAKE_CURRENT_BINARY_DIR}/tk/wish_runAllDemos
	WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/tk
	VERBATIM
	)
      add_dependencies(test_wish_runAllDemos
	${plplottcltk_target}
	${tkwin_targets}
	tcl_examples
	${TARGET_tclIndex_tcl}
	${TARGET_tclIndex_tk}
	tclIndex_examples_tk
	)
      message(STATUS "WARNING: The test_wish_runAllDemos target can be run independently but
   there are several issues with this target so it is temporarily excluded from being a
   dependency of other more general interactive test targets")
      # list(APPEND targets_examples_tk test_wish_runAllDemos)
    endif(BUILD_SHARED_LIBS AND PLD_tkwin)
  endif(ENABLE_tkX AND xwin_targets)

  if(targets_examples_tk)
    list(APPEND interactive_targets_LIST ${targets_examples_tk})
    list(APPEND tk_targets_LIST ${targets_examples_tk})
  endif(targets_examples_tk)

  add_custom_target(test_interactive)
  if(interactive_targets_LIST)
    add_dependencies(test_interactive ${interactive_targets_LIST})
  endif(interactive_targets_LIST)

  add_custom_target(test_tcl)
  if(tcl_targets_LIST)
    add_dependencies(test_tcl ${tcl_targets_LIST})
  endif(tcl_targets_LIST)

  add_custom_target(test_tk)
  if(tk_targets_LIST)
    add_dependencies(test_tk ${tk_targets_LIST})
  endif(tk_targets_LIST)

  # List of custom targets that test all of qt devices/special examples.
  # Must be maintained.
  set(qt_test_target_LIST
    test_c_bmpqt
    test_c_epsqt
    test_c_jpgqt
    test_c_pdfqt
    test_c_pngqt
    test_c_ppmqt
    test_c_qtwidget
    test_c_svgqt
    test_c_tiffqt
    #test_octave_qtwidget
    test_qt_example
    )
  if(ENABLE_pyqt4)
    list(APPEND qt_test_target_LIST test_pyqt4_example)
  elseif(ENABLE_pyqt5)
    list(APPEND qt_test_target_LIST test_pyqt5_example)
  endif(ENABLE_pyqt4)

  set(qt_test_target_DEPENDS)
  foreach(qt_test_target ${qt_test_target_LIST})
    if(TARGET ${qt_test_target})
      list(APPEND qt_test_target_DEPENDS ${qt_test_targets})
    endif(TARGET ${qt_test_target})
  endforeach(qt_test_target ${qt_test_target_LIST})
  if(qt_test_target_DEPENDS)
    add_custom_target(test_all_qt)
    add_dependencies(test_all_qt ${qt_test_target_DEPENDS})
  endif(qt_test_target_DEPENDS)

  # List of custom targets that test all of cairo devices/special examples.
  # Must be maintained.
  set(cairo_test_target_LIST
    test_c_pdfcairo
    test_c_pngcairo
    test_c_pscairo
    test_c_epscairo
    test_c_svgcairo
    test_extcairo
    test_c_xcairo
    test_extXdrawable
    )

  set(cairo_test_target_DEPENDS)
  foreach(cairo_test_target ${cairo_test_target_LIST})
    if(TARGET ${cairo_test_target})
      list(APPEND cairo_test_target_DEPENDS ${cairo_test_target})
    endif(TARGET ${cairo_test_target})
  endforeach(cairo_test_target ${cairo_test_target_LIST})
  if(cairo_test_target_DEPENDS)
    add_custom_target(test_all_cairo)
    add_dependencies(test_all_cairo ${cairo_test_target_DEPENDS})
  endif(cairo_test_target_DEPENDS)

endif(BUILD_TEST)

if(NOT CORE_BUILD)
  set(summary_results "
Summary of CMake build system results for the installed examples

Noninteractive device drivers:

PLD_pdfcairo:		${PLD_pdfcairo}
PLD_pngcairo:		${PLD_pngcairo}
PLD_pscairo:		${PLD_pscairo}
PLD_epscairo:		${PLD_epscairo}
PLD_svgcairo:		${PLD_svgcairo}
PLD_cgm:		${PLD_cgm}
PLD_epsqt:		${PLD_epsqt}
PLD_pdfqt:		${PLD_pdfqt}
PLD_bmpqt:		${PLD_bmpqt}
PLD_jpgqt:		${PLD_jpgqt}
PLD_pngqt:		${PLD_pngqt}
PLD_ppmqt:		${PLD_ppmqt}
PLD_tiffqt:		${PLD_tiffqt}
PLD_svgqt:		${PLD_svgqt}
PLD_gif:		${PLD_gif}
PLD_jpeg:		${PLD_jpeg}
PLD_png:		${PLD_png}
PLD_pdf:		${PLD_pdf}
PLD_ps:			${PLD_ps}
PLD_psc:		${PLD_psc}
PLD_pstex:		${PLD_pstex}
PLD_psttf:		${PLD_psttf}
PLD_psttfc:		${PLD_psttfc}
PLD_svg:		${PLD_svg}
PLD_wxpng:		${PLD_wxpng}
PLD_xfig:		${PLD_xfig}

Interactive device drivers:

PLD_xcairo:		${PLD_xcairo}
PLD_extcairo:		${PLD_extcairo}

N.B. The above devices are all that are currently configurable by the
installed examples build system.  Additional interactive device
drivers are likely supplied by the PLplot core and configured as part
of plplot-test-interactive.sh that is run by

make test_interactive


but the build system for the installed examples cannot configure anything
with those device drivers so we don't bother to retrieve them from the
core build system or output them here.

Noninteractive bindings:

ENABLE_ada:		${ENABLE_ada}
ENABLE_c:		${ENABLE_c}
ENABLE_cxx:		${ENABLE_cxx}
ENABLE_d:		${ENABLE_d}
ENABLE_fortran:		${ENABLE_fortran}
ENABLE_java:		${ENABLE_java}
ENABLE_lua:		${ENABLE_lua}
ENABLE_ocaml:		${ENABLE_ocaml}
ENABLE_octave:		${ENABLE_octave}
ENABLE_python:		${ENABLE_python}
ENABLE_tcl:		${ENABLE_tcl}

Interactive bindings:

ENABLE_qt:		${ENABLE_qt}
ENABLE_tk:		${ENABLE_tk}
ENABLE_wxwidgets:	${ENABLE_wxwidgets}
")

  message("${summary_results}")

endif(NOT CORE_BUILD)
