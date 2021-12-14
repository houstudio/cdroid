# bindings/tcl/CMakeLists.txt
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

if(ENABLE_tcl)

  # Check consistency of plplot_parameters.h.
  add_custom_target(
    check_tcl_parameters
    COMMAND ${CMAKE_COMMAND} -E echo "Check that bindings/tcl/plplot_parameters.h is consistent with the #defines in bindings/swig-support/plplotcapi.i"
    COMMAND ${CMAKE_COMMAND} -E remove -f
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_parameters.h_compare
    COMMAND
    sed -f ${CMAKE_CURRENT_SOURCE_DIR}/global_defines.sed <
    ${PROJECT_SOURCE_DIR}/bindings/swig-support/plplotcapi.i >
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_parameters.h_compare
    COMMAND
    cmp ${CMAKE_CURRENT_SOURCE_DIR}/plplot_parameters.h
    ${CMAKE_CURRENT_BINARY_DIR}/plplot_parameters.h_compare
    )

  # check_plplotcapi_defines should be done first since
  # ${PROJECT_SOURCE_DIR}/bindings/swig-support/plplotcapi.i
  # might be changed as a result of that check.
  add_dependencies(check_tcl_parameters check_plplotcapi_defines)
  add_dependencies(check_all check_tcl_parameters)

  include_directories(
    ${TCL_INCLUDE_PATH}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/lib/qsastime
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/lib/qsastime
    )

  # These installed header files required regardless of whether the
  # tclmatrix and plplottcltk libraries are built or their source
  # code is included in the plplot library.
  set(tclmatrix_INSTALLED_HEADERS
    tclMatrix.h
    )

  install(
    FILES ${tclmatrix_INSTALLED_HEADERS}
    DESTINATION ${INCLUDE_DIR}
    )

  set(plplottcltk_INSTALLED_HEADERS
    pltcl.h
    )

  install(
    FILES ${plplottcltk_INSTALLED_HEADERS}
    DESTINATION ${INCLUDE_DIR}
    )

  if(NOT tcltk_in_plplot_library)
    # tclmatrix
    set(tclmatrix_LIB_SRCS
      tclMatrix.c
      matrixInit.c
      )

    set(
      libplplottcltk_COMPILE_FLAGS
      "-I\"${TCL_INCLUDE_PATH}\""
      )

    if(USE_TCL_TK_STUBS)
      set(libtclmatrix_LINK_LIBRARIES ${TCL_STUB_LIBRARY})
      set_source_files_properties(
	${tclmatrix_LIB_SRCS}
	PROPERTIES COMPILE_DEFINITIONS USE_TCL_STUBS
	)
    else(USE_TCL_TK_STUBS)
      set(libtclmatrix_LINK_LIBRARIES ${TCL_LIBRARY})
    endif(USE_TCL_TK_STUBS)

    # tclmatrix depends on the Tcl library.
    set(LIB_INSTALL_RPATH ${TCL_RPATH})
    configure_library_build(tclmatrix "" "${tclmatrix_LIB_SRCS}" "${libtclmatrix_LINK_LIBRARIES}" "${LIB_INSTALL_RPATH}")

    # end tclmatrix

    # plplottcltk
    set(
      tclgenfiles
      ${CMAKE_CURRENT_BINARY_DIR}/tclgen.h
      ${CMAKE_CURRENT_BINARY_DIR}/tclgen_s.h
      ${CMAKE_CURRENT_BINARY_DIR}/tclgen.c
      )

    add_custom_command(
      OUTPUT ${tclgenfiles}
      COMMAND ${TCL_TCLSH}
      ${CMAKE_CURRENT_SOURCE_DIR}/pltclgen.tcl ${CMAKE_CURRENT_SOURCE_DIR}
      DEPENDS
      ${CMAKE_CURRENT_SOURCE_DIR}/pltclgen.tcl
      ${CMAKE_CURRENT_SOURCE_DIR}/plapi.tpl
      ${CMAKE_CURRENT_SOURCE_DIR}/tclcmd.tpl
      )

    include_directories(${CMAKE_CURRENT_BINARY_DIR})
    set_source_files_properties(
      tclAPI.c
      PROPERTIES OBJECT_DEPENDS "${tclgenfiles}"
      )

    set(plplottcltk_LIB_SRCS
      tclAPI.c
      )

    if(USE_TCL_TK_STUBS)
      set(
	plplottcltk_link_libraries
	PLPLOT::tclmatrix
	PLPLOT::plplot
	${TCL_STUB_LIBRARY}
	)
      set(
	libplplottcltk_LINK_FLAGS
	-ltclmatrix
	${TCL_STUB_LIBRARY}
	)
    else(USE_TCL_TK_STUBS)
      set(
	plplottcltk_link_libraries
	PLPLOT::tclmatrix
	PLPLOT::plplot
	${TCL_LIBRARY}
	)
      set(
	libplplottcltk_LINK_FLAGS
	-ltclmatrix
	${TCL_LIBRARY}
	)
    endif(USE_TCL_TK_STUBS)

    if(ENABLE_tkX)
      set(plplottcltk_LIB_SRCS
	${plplottcltk_LIB_SRCS}
	${PROJECT_SOURCE_DIR}/bindings/tk/Pltk_Init.c
	${PROJECT_SOURCE_DIR}/bindings/tk/plframe.c
	${PROJECT_SOURCE_DIR}/bindings/tk/plr.c
	${PROJECT_SOURCE_DIR}/bindings/tk/tcpip.c
	)

      include_directories(${TK_INCLUDE_PATH})
      # Transform TK_INCLUDE_PATH (which is a list) to blank-delimited flag form.
      string(REGEX REPLACE ";" "\" -I\"" TKLIB_COMPILE_FLAGS "-I\"${TK_INCLUDE_PATH}\"")
      set(
	libplplottcltk_COMPILE_FLAGS
	"${libplplottcltk_COMPILE_FLAGS} ${TKLIB_COMPILE_FLAGS}"
	)

      if(USE_TCL_TK_STUBS)
	set(plplottcltk_link_libraries
	  ${plplottcltk_link_libraries}
	  ${TK_STUB_LIBRARY}
	  ${X11_X11_LIB}
	  )
	set(
	  libplplottcltk_LINK_FLAGS
	  ${libplplottcltk_LINK_FLAGS}
	  ${TK_STUB_LIBRARY}
	  )
      else(USE_TCL_TK_STUBS)
	set(plplottcltk_link_libraries
	  ${plplottcltk_link_libraries}
	  ${TK_LIBRARY}
	  ${X11_X11_LIB}
	  )
	set(
	  libplplottcltk_LINK_FLAGS
	  ${libplplottcltk_LINK_FLAGS}
	  ${TK_LIBRARY}
	  )
      endif(USE_TCL_TK_STUBS)
    endif(ENABLE_tkX)

    if(USE_TCL_TK_STUBS)
      if(ENABLE_tkX)
	set_source_files_properties(
	  ${plplottcltk_LIB_SRCS}
	  PROPERTIES COMPILE_DEFINITIONS "USE_TCL_STUBS;USE_TK_STUBS"
	  )
      else(ENABLE_tkX)
	set_source_files_properties(
	  ${plplottcltk_LIB_SRCS}
	  PROPERTIES COMPILE_DEFINITIONS USE_TCL_STUBS
	  )
      endif(ENABLE_tkX)
    endif(USE_TCL_TK_STUBS)

    #message(STATUS "plplottcltk_link_libraries = ${plplottcltk_link_libraries}"

    # plplottcltk depends on the plplot library.
    set(LIB_INSTALL_RPATH ${LIB_DIR})
    # plplottcltk depends on the Tcl and Tk libraries.
    list(APPEND LIB_INSTALL_RPATH ${TCL_TK_RPATH})
    configure_library_build(plplottcltk "" "${plplottcltk_LIB_SRCS}" "${plplottcltk_link_libraries}" "${LIB_INSTALL_RPATH}")

    pkg_config_file("tcl" "Tcl/Tk" " Tcl/Tk bindings" "plplottcltk" "${libplplottcltk_COMPILE_FLAGS}" "-lplplot;${libplplottcltk_LINK_FLAGS}")
    # end plplottcltk
  endif(NOT tcltk_in_plplot_library)

  # start plplottcltk_Main

  set(plplottcltk_Main_LIB_SRCS
    tclMain.c
    )

  set(
    plplottcltk_Main_link_libraries
    PLPLOT::plplot
    ${TCL_LIBRARY}
    )
  set(
    libplplottcltk_Main_LINK_FLAGS
    ${TCL_LIBRARY}
    )

  message(STATUS "ENABLE_itcl: ${ENABLE_itcl}")
  if(ENABLE_itcl)
    include_directories(${ITCL_INCLUDE_PATH})

    # tkMain.c is the only code that currently has a dependence on
    # itcl library routines.  Since that code is now part of the source code
    # for libplplottcltk_Main, only flags for that library (but not
    # libplplottcltk any more) are modified below.

    set(
      libplplottcltk_Main_COMPILE_FLAGS
      "${libplplottcltk_Main_COMPILE_FLAGS} -I\"${ITCL_INCLUDE_PATH}\""
      )
    set(plplottcltk_Main_link_libraries
      ${plplottcltk_Main_link_libraries}
      ${ITCL_LIBRARY}
      )
    set(
      libplplottcltk_Main_LINK_FLAGS
      ${libplplottcltk_Main_LINK_FLAGS}
      ${ITCL_LIBRARY}
      )
  endif(ENABLE_itcl)
  message(STATUS "Itcl libraries: ${plplottcltk_Main_link_libraries}")

  if(ENABLE_tkX)

    set(plplottcltk_Main_LIB_SRCS
      ${plplottcltk_Main_LIB_SRCS}
      ${PROJECT_SOURCE_DIR}/bindings/tk/tkMain.c
      )

    include_directories(${TK_INCLUDE_PATH})
    # Transform TK_INCLUDE_PATH (which is a list) to blank-delimited flag form.
    string(REGEX REPLACE ";" "\" -I\"" TKLIB_COMPILE_FLAGS "-I\"${TK_INCLUDE_PATH}\"")

    set(
      libplplottcltk_Main_COMPILE_FLAGS
      "${libplplottcltk_Main_COMPILE_FLAGS} ${TKLIB_COMPILE_FLAGS}"
      )

    set(plplottcltk_Main_link_libraries
      ${plplottcltk_Main_link_libraries}
      ${TK_LIBRARY}
      ${X11_X11_LIB}
      )
    set(
      libplplottcltk_Main_LINK_FLAGS
      ${libplplottcltk_Main_LINK_FLAGS}
      ${TK_LIBRARY}
      )

    if(ENABLE_itkX)
      include_directories(${ITK_INCLUDE_PATH})
      # tkMain.c is the only code that currently has a dependence on
      # itk library routines.  Since that code is now part of the source code
      # for libplplottcltk_Main, only flags for that library (but not
      # libplplottcltk any more) are modified below.

      set(
	libplplottcltk_Main_COMPILE_FLAGS
	"${libplplottcltk_Main_COMPILE_FLAGS} -I\"${ITK_INCLUDE_PATH}\""
	)
      set(plplottcltk_Main_link_libraries
	${plplottcltk_Main_link_libraries}
	${ITK_LIBRARY}
	)
      set(
	libplplottcltk_Main_LINK_FLAGS
	${libplplottcltk_Main_LINK_FLAGS}
	${ITK_LIBRARY}
	)
    endif(ENABLE_itkX)
  endif(ENABLE_tkX)

  # This LIB_INSTALL_RPATH used as well below for pltcl.
  # plplottcltk_Main depends on the plplot library.
  set(LIB_INSTALL_RPATH ${LIB_DIR})
  # plplottcltk_Main depends on the Tcl/Tk/Itcl/Itk libraries.
  list(APPEND LIB_INSTALL_RPATH ${TCL_TK_ITCL_ITK_RPATH})

  configure_library_build(plplottcltk_Main "" "${plplottcltk_Main_LIB_SRCS}" "${plplottcltk_Main_link_libraries}" "${LIB_INSTALL_RPATH}")

  # end plplottcltk_Main

  # Generate plplot.tcl and tclIndex in build tree.  At least the first one
  # is always needed for build-tree tests of tcl examples.

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/plplot.tcl
    COMMAND ${CMAKE_COMMAND} -E copy
    ${PROJECT_SOURCE_DIR}/bindings/tk/plplot.tcl
    ${CMAKE_CURRENT_BINARY_DIR}/plplot.tcl
    DEPENDS
    ${PROJECT_SOURCE_DIR}/bindings/tk/plplot.tcl
    )

  # _target suffix to avoid nameclash with filename plplot.tcl.  This
  # nameclash screwed up ../tk/configuration which had a non-generated file
  # of the same name which was a dependency of a custom target.
  add_custom_target(plplot.tcl_target ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/plplot.tcl
    )
  set_property(GLOBAL PROPERTY
    FILES_plplot.tcl_target ${CMAKE_CURRENT_BINARY_DIR}/plplot.tcl
    )

  # A Tcl expert should insert the file depends here.
  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
    COMMAND ${TCL_TCLSH} ${MKTCLINDEX} ${MKTCLINDEX_ARGS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  add_custom_target(tclIndex_tcl ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
    )
  set_property(GLOBAL PROPERTY
    FILES_tclIndex_tcl ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
    )

  # I don't think this is necessary, but I am not sure of the file
  # depends of the custom_command associated with tclIndex_tcl and
  # serializing these targets doesn't slow down parallel builds very
  # much.
  add_dependencies(tclIndex_tcl plplot.tcl_target)

  if(NOT ENABLE_tk)
    # Only install these two files if the equivalent files are not going to
    # be installed by the action of ../tk/CMakeLists.txt

    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/plplot.tcl
      DESTINATION ${TCL_DIR}
      )

    install(
      FILES ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
      DESTINATION ${TCL_DIR}
      )
  endif(NOT ENABLE_tk)

  if(USE_DLL_SUBDIRECTORY)
    set(LIB_INSTALL_DIR ${BIN_DIR})
  else(USE_DLL_SUBDIRECTORY)
    set(LIB_INSTALL_DIR ${LIB_DIR})
  endif(USE_DLL_SUBDIRECTORY)

  if(tcltk_in_plplot_library)
    set(DLNAME_LIBRARY_DIR $<TARGET_FILE_DIR:PLPLOT::plplot>)
    set(DLNAME_LIBRARY $<TARGET_FILE_NAME:PLPLOT::plplot>)
  else(tcltk_in_plplot_library)
    set(DLNAME_LIBRARY_DIR $<TARGET_FILE_DIR:PLPLOT::plplottcltk>)
    set(DLNAME_LIBRARY $<TARGET_FILE_NAME:PLPLOT::plplottcltk>)
  endif(tcltk_in_plplot_library)

  # N.B. Results only available at generate time.
  configure_file_generate(
    ${CMAKE_CURRENT_SOURCE_DIR}/pkgIndex.tcl.in
    ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
    @ONLY
    )

  pkg_config_file("tcl_Main" "Tcl_Main/Tk_Main" " Tcl_Main/Tk_Main bindings" "plplottcltk_Main" "${libplplottcltk_Main_COMPILE_FLAGS}" "-lplplot;${libplplottcltk_Main_LINK_FLAGS}")

  set(pltcl_link_libraries)
  if(tcltk_in_plplot_library)
    list(APPEND pltcl_link_libraries PLPLOT::plplottcltk_Main PLPLOT::plplot)
  else(tcltk_in_plplot_library)
    list(APPEND pltcl_link_libraries PLPLOT::plplottcltk_Main PLPLOT::plplottcltk PLPLOT::plplot)
  endif(tcltk_in_plplot_library)
  if(ENABLE_itcl)
    list(APPEND pltcl_link_libraries ${ITCL_LIBRARY} ${TCL_LIBRARY})
  else(ENABLE_itcl)
    list(APPEND pltcl_link_libraries ${TCL_LIBRARY})
  endif(ENABLE_itcl)

  configure_executable_build(pltcl "pltcl.c" "${pltcl_link_libraries}" "${LIB_INSTALL_RPATH}" ${BIN_DIR})

  # test_tclmatrix.sh is a configured shell script that tests
  # the matrix capabilities of the tclmatrix library using
  # the tcl code in test_tclmatrix.tcl.
  set(pltcl_LOCATION $<TARGET_FILE:PLPLOT::pltcl>)
  # N.B. Results only available at generate time.
  configure_file_generate(
    ${CMAKE_CURRENT_SOURCE_DIR}/test_tclmatrix.sh.in
    ${CMAKE_CURRENT_BINARY_DIR}/test_tclmatrix.sh
    @ONLY
    )

  # Configure a target to check the matrix capabilities of libtclmatrix using pltcl.

  add_custom_target(
    check_libtclmatrix_capabilities
    COMMAND ${CMAKE_COMMAND} -E echo "Check the matrix capabilities of libtclmatrix"
    COMMAND ${CMAKE_COMMAND} -E remove -f
    ${CMAKE_CURRENT_BINARY_DIR}/test_tclmatrix.out_compare
    COMMAND env ${CMAKE_CURRENT_BINARY_DIR}/test_tclmatrix.sh > ${CMAKE_CURRENT_BINARY_DIR}/test_tclmatrix.out_compare
    COMMAND
    cmp ${CMAKE_CURRENT_SOURCE_DIR}/test_tclmatrix.out
    ${CMAKE_CURRENT_BINARY_DIR}/test_tclmatrix.out_compare
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )
  add_dependencies(check_libtclmatrix_capabilities PLPLOT::pltcl tclIndex_tcl)
  add_dependencies(check_all check_libtclmatrix_capabilities)

endif(ENABLE_tcl)
