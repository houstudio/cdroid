# bindings/tk/CMakeLists.txt
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

if(ENABLE_tkX)
  include_directories(
    ${TCL_INCLUDE_PATH}
    ${TK_INCLUDE_PATH}
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_SOURCE_DIR}/bindings/tcl
    ${PROJECT_SOURCE_DIR}/lib/qsastime
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${PROJECT_BINARY_DIR}
    ${PROJECT_BINARY_DIR}/include
    ${PROJECT_BINARY_DIR}/lib/qsastime
    )

  if(tcltk_in_plplot_library)
    set(plserver_link_libraries PLPLOT::plplot PLPLOT::plplottcltk_Main ${TK_LIBRARY} ${TCL_LIBRARY})
  else(tcltk_in_plplot_library)
    set(plserver_link_libraries PLPLOT::plplot PLPLOT::plplottcltk_Main PLPLOT::plplottcltk ${TK_LIBRARY} ${TCL_LIBRARY})
  endif(tcltk_in_plplot_library)

  # plserver depends on the plplot library.
  set(LIB_INSTALL_RPATH ${LIB_DIR})
  # plserver depends on the Tcl/Tk/Itcl/Itk libraries
  list(APPEND LIB_INSTALL_RPATH ${TCL_TK_ITCL_ITK_RPATH})
  configure_executable_build(plserver "plserver.c" "${plserver_link_libraries}" "${LIB_INSTALL_RPATH}" ${BIN_DIR})
endif(ENABLE_tkX)

if(ENABLE_tk)

  set(include_INSTALLED_HEADERS
    pltk.h
    )

  install(FILES ${include_INSTALLED_HEADERS} DESTINATION ${INCLUDE_DIR})

  set(tcldata
    FileSelector.tcl
    Pltkwin.tcl
    about.tcl
    help_gui.tcl
    help_keys.tcl
    help_tcltk.tcl
    plclient.tcl
    plcolor.tcl
    plconfig.tcl
    pldefaults.tcl
    plplot.tcl
    plserver.tcl
    pltools.tcl
    plwidget.tcl
    )

  set(itkdata
    PLWin.itk
    PLXWin.itk
    )

  set(paldata
    cmap0a.pal
    cmap1a.pal
    cmap1a1.pal
    cmap1b.pal
    cmap1c.pal
    cmap1d.pal
    )

  # Ugly hack to copy all .tcl files to the build tree so that tclIndex
  # can be generated there. This is a limitation is tcl's auto_mkindex which
  # always generates the index in the directory with the files.
  if(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(tclIndex_DEPENDS)
    foreach(file ${tcldata} ${itkdata})
      set(
	tclIndex_DEPENDS
	${tclIndex_DEPENDS}
	${CMAKE_CURRENT_BINARY_DIR}/${file}
	)
      add_custom_command(
	OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${file}
	COMMAND ${CMAKE_COMMAND} -E copy
	${CMAKE_CURRENT_SOURCE_DIR}/${file} ${CMAKE_CURRENT_BINARY_DIR}
	DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${file}
	)
    endforeach(file ${tcldata} ${itkdata})
    add_custom_target(tcl_files ALL DEPENDS ${tclIndex_DEPENDS})
  else(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    set(tclIndex_DEPENDS ${tcldata} ${itkdata})
  endif(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

  install(
    FILES ${tcldata} ${itkdata} ${paldata}
    DESTINATION ${TCL_DIR}
    )

  add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
    COMMAND ${TCL_TCLSH} ${MKTCLINDEX} ${MKTCLINDEX_ARGS}
    DEPENDS ${tclIndex_DEPENDS}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    )

  add_custom_target(tclIndex_tk ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
    )

  # The tcl_files dependency is required to
  # work around CMake 2-level make recursion issue where independent
  # file dependency chains that refer to the same files must have
  # target dependencies between them in order to work for parallel
  # builds (and possibly some visual studio builds).
  if(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
    add_dependencies(tclIndex_tk tcl_files)
  endif(NOT CMAKE_CURRENT_BINARY_DIR STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

  install(
    FILES ${CMAKE_CURRENT_BINARY_DIR}/tclIndex
    DESTINATION ${TCL_DIR}
    )
endif(ENABLE_tk)

if(PLD_tk)
  if(USE_DLL_SUBDIRECTORY)
    set(LIB_INSTALL_DIR ${BIN_DIR})
  else(USE_DLL_SUBDIRECTORY)
    set(LIB_INSTALL_DIR ${DRV_DIR})
  endif(USE_DLL_SUBDIRECTORY)

  if(tcltk_in_plplot_library)
    set(DLNAME_LIBRARY_DIR $<TARGET_FILE_DIR:PLPLOT::plplot>)
    set(DLNAME_LIBRARY $<TARGET_FILE_NAME:PLPLOT::plplot>)
  else(tcltk_in_plplot_library)
    set(DLNAME_LIBRARY_DIR $<TARGET_FILE_DIR:PLPLOT::tk>)
    set(DLNAME_LIBRARY $<TARGET_FILE_NAME:PLPLOT::tk>)
  endif(tcltk_in_plplot_library)

  # N.B. Results only available at generate time.
  configure_file_generate(
    ${CMAKE_CURRENT_SOURCE_DIR}/pkgIndex.tcl.in
    ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
    @ONLY
    )
endif(PLD_tk)

