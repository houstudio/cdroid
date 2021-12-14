# src/CMakeLists.txt for PLplot
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

set(plplot_LIB_SRCS
  pdfutils.c
  plmem.c
  plaffine.c
  plarc.c
  plargs.c
  plbox.c
  plcont.c
  plcore.c
  plctrl.c
  plcvt.c
  pldtik.c
  plf2ops.c
  plfill.c
  plfreetype.c
  plgradient.c
  plhist.c
  plimage.c
  plline.c
  plmap.c
  plmetafile.c
  plot3d.c
  plpage.c
  plsdef.c
  plshade.c
  plstdio.c
  plstripc.c
  plsym.c
  pltick.c
  plvpor.c
  plwind.c
  plbuf.c
  plgridd.c
  plvect.c
  mt19937ar.c
  pltime.c
  pllegend.c
  )
if(PL_DEPRECATED)
  list(APPEND plplot_LIB_SRCS pldeprecated.c)
endif(PL_DEPRECATED)
if(LTDL_WIN32)
  list(APPEND plplot_LIB_SRCS ltdl_win32.c)
endif(LTDL_WIN32)

option(HERSHEY_FALLBACK "Hershey fallback for Unicode fonts" ON)

if(NOT HERSHEY_FALLBACK)
  get_source_file_property(PLSYM_COMPILE_PROPS plsym.c COMPILE_FLAGS)
  if(NOT PLSYM_COMPILE_PROPS)
    set(PLSYM_COMPILE_PROPS)
  endif(NOT PLSYM_COMPILE_PROPS)
  set_source_files_properties(
    plsym.c
    PROPERTIES COMPILE_FLAGS "${PLSYM_COMPILE_PROPS} -DPL_TEST_FOR_MISSING_GLYPHS"
    )
endif(NOT HERSHEY_FALLBACK)

set(pc_libplplot_COMPILE_FLAGS)

get_source_file_property(PLCORE_COMPILE_PROPS plcore.c COMPILE_FLAGS)
# Deal with NOTFOUND case.
if(NOT PLCORE_COMPILE_PROPS)
  set(PLCORE_COMPILE_PROPS)
endif(NOT PLCORE_COMPILE_PROPS)
if(WIN32_OR_CYGWIN)
  set(PLCORE_COMPILE_PROPS "${PLCORE_COMPILE_PROPS} -DIGNORECASE")
endif(WIN32_OR_CYGWIN)

if(ENABLE_DYNDRIVERS)
  set(PLCORE_COMPILE_PROPS "${PLCORE_COMPILE_PROPS} -I${LTDL_INCLUDE_DIR}")
endif(ENABLE_DYNDRIVERS)

set_source_files_properties(
  plcore.c
  PROPERTIES COMPILE_FLAGS "${PLCORE_COMPILE_PROPS}"
  )

if(NOT ENABLE_DYNDRIVERS)
  if(ENABLE_tcl)
    set(
      tclgenfiles
      ${PROJECT_BINARY_DIR}/bindings/tcl/tclgen.h
      ${PROJECT_BINARY_DIR}/bindings/tcl/tclgen_s.h
      ${PROJECT_BINARY_DIR}/bindings/tcl/tclgen.c
      )

    # Only need to set up this special command if either the tk and tkwin
    # non-dynamic devices are enabled (see below) since at build time
    # this special command is only fired in that case.
    add_custom_command(
      OUTPUT ${tclgenfiles}
      COMMAND ${TCL_TCLSH}
      ${PROJECT_SOURCE_DIR}/bindings/tcl/pltclgen.tcl
      ${PROJECT_SOURCE_DIR}/bindings/tcl
      DEPENDS
      ${PROJECT_SOURCE_DIR}/bindings/tcl/pltclgen.tcl
      ${PROJECT_SOURCE_DIR}/bindings/tcl/plapi.tpl
      ${PROJECT_SOURCE_DIR}/bindings/tcl/tclcmd.tpl
      WORKING_DIRECTORY ${PROJECT_BINARY_DIR}/bindings/tcl
      )
  endif(ENABLE_tcl)

  # Nondynamic device drivers must be included in the core PLplot library.
  set(DRIVERS_RPATH)
  foreach(SOURCE_ROOT_NAME ${DRIVERS_LIST})
    #message("${SOURCE_ROOT_NAME}_SOURCE = ${${SOURCE_ROOT_NAME}_SOURCE}")
    list(APPEND plplot_LIB_SRCS ${${SOURCE_ROOT_NAME}_SOURCE})

    if(SOURCE_ROOT_NAME STREQUAL "qt")
      if(ANY_QT_DEVICE)
	# Must add qt.h to explicit sources for AUTOMOC method to work for either
	# Qt4 or Qt5.
	list(APPEND plplot_LIB_SRCS ${PROJECT_SOURCE_DIR}/include/qt.h)
	list(APPEND pc_libplplot_COMPILE_FLAGS ${pc_qt_COMPILE_FLAGS})

      else(ANY_QT_DEVICE)
	message(FATAL_ERROR "Internal build system inconsistency.  Attempt to build static qt device when ANY_QT_DEVICE is false.")
      endif(ANY_QT_DEVICE)
    endif(SOURCE_ROOT_NAME STREQUAL "qt")

    if(${SOURCE_ROOT_NAME}_COMPILE_FLAGS)
      set_source_files_properties(
	${${SOURCE_ROOT_NAME}_SOURCE}
	PROPERTIES COMPILE_FLAGS ${${SOURCE_ROOT_NAME}_COMPILE_FLAGS}
	)
      list(APPEND pc_libplplot_COMPILE_FLAGS ${${SOURCE_ROOT_NAME}_COMPILE_FLAGS})
      if(SOURCE_ROOT_NAME STREQUAL "tk")
        set_source_files_properties(
	  ${${SOURCE_ROOT_NAME}_SOURCE}
	  PROPERTIES OBJECT_DEPENDS ${tclgenfiles}
	  )
      endif(SOURCE_ROOT_NAME STREQUAL "tk")
      if(SOURCE_ROOT_NAME STREQUAL "tkwin")
        set_source_files_properties(
	  ${${SOURCE_ROOT_NAME}_SOURCE}
	  PROPERTIES OBJECT_DEPENDS ${tclgenfiles}
	  )
      endif(SOURCE_ROOT_NAME STREQUAL "tkwin")
    endif(${SOURCE_ROOT_NAME}_COMPILE_FLAGS)
    if(${SOURCE_ROOT_NAME}_RPATH)
      # Accumulate driver rpath information into DRIVERS_RPATH.
      list(APPEND DRIVERS_RPATH ${${SOURCE_ROOT_NAME}_RPATH})
    endif(${SOURCE_ROOT_NAME}_RPATH)
  endforeach(SOURCE_ROOT_NAME ${DRIVERS_LIST})
endif(NOT ENABLE_DYNDRIVERS)

include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/lib/qsastime
  ${PROJECT_BINARY_DIR}
  ${PROJECT_BINARY_DIR}/include
  ${PROJECT_BINARY_DIR}/lib/qsastime
  )

# The plplot library depends on the "lib subdirectory" libraries such
# as csironn, csirocsa, qsastime, and (possibly) nistcd.
set(LIB_INSTALL_RPATH ${LIB_DIR})
set(libplplot_LINK_LIBRARIES)
if(ENABLE_DYNDRIVERS)
  list(APPEND libplplot_LINK_LIBRARIES ${LTDL_LIBRARIES})
  #message("libplplot_LINK_LIBRARIES =  ${libplplot_LINK_LIBRARIES}")
  #message("LTDL_LIBRARY_RPATH = ${libplplot_RPATH}")
  # For this case the plplot library depends on the ltdl library.
  list(APPEND LIB_INSTALL_RPATH ${LTDL_LIBRARY_RPATH})
else(ENABLE_DYNDRIVERS)
  list(APPEND libplplot_LINK_LIBRARIES ${DRIVERS_LINK_FLAGS})
  # For this case the plplot library depends on external libraries required
  # by the device driver code that is in the plplot library.
  list(APPEND LIB_INSTALL_RPATH ${DRIVERS_RPATH})
endif(ENABLE_DYNDRIVERS)
#message("DEBUG: LIB_INSTALL_RPATH = ${LIB_INSTALL_RPATH}")

if(MATH_LIB)
  list(APPEND libplplot_LINK_LIBRARIES ${MATH_LIB})
endif(MATH_LIB)

if(HAVE_SHAPELIB)
  get_source_file_property(PLMAP_COMPILE_PROPS plmap.c COMPILE_FLAGS)
  # Deal with NOTFOUND case.
  if(NOT PLMAP_COMPILE_PROPS)
    set(PLMAP_COMPILE_PROPS)
  endif(NOT PLMAP_COMPILE_PROPS)
  set_source_files_properties(plmap.c
    PROPERTIES
    COMPILE_FLAGS "${PLMAP_COMPILE_PROPS} -I${SHAPELIB_INCLUDE_DIR}"
    )
  if(HAVE_SAHOOKS)
  set_source_files_properties(plmap.c
    PROPERTIES
    COMPILE_DEFINITIONS HAVE_SAHOOKS
    )
  endif(HAVE_SAHOOKS)

  list(APPEND libplplot_LINK_LIBRARIES ${SHAPELIB_LIBRARIES})
  # The plplot library depends on shapelib.
  list(APPEND LIB_INSTALL_RPATH ${SHAPELIB_RPATH})

endif(HAVE_SHAPELIB)

if(WITH_FREETYPE)
  get_source_file_property(PLFREETYPE_COMPILE_PROPS plfreetype.c COMPILE_FLAGS)
  # Deal with NOTFOUND case.
  if(NOT PLFREETYPE_COMPILE_PROPS)
    set(PLFREETYPE_COMPILE_PROPS)
  endif(NOT PLFREETYPE_COMPILE_PROPS)
  set_source_files_properties(plfreetype.c PROPERTIES COMPILE_FLAGS "${PLFREETYPE_COMPILE_PROPS} ${FREETYPE_INCLUDE_CFLAGS}")
  list(APPEND libplplot_LINK_LIBRARIES ${FREETYPE_LIBRARIES})
endif(WITH_FREETYPE)

# Copy current information in libplplot_LINK_LIBRARIES
# into pc_libplplot_LINK_FLAGS, which contains the equivalent list information
# in a combination of library list form and pkg-config form that will
# eventually be transformted into pure pkg-config form.  Both variables
# will have data added to them afterward due to the internal PLplot libraries
# which have a different pkg-config treatment than the external libraries
# dealt with here.
set(pc_libplplot_LINK_FLAGS ${libplplot_LINK_LIBRARIES})

if(NOT ENABLE_DYNDRIVERS AND ANY_QT_DEVICE AND PLPLOT_USE_QT5)
  # N.B. pc_qt_LIBRARIES_LIST is the same as QT_LIBRARIES for
  # the Qt4 case which is why this logic is redundant in that
  # case.
  list(APPEND pc_libplplot_LINK_FLAGS ${pc_qt_LIBRARIES_LIST})
endif(NOT ENABLE_DYNDRIVERS AND ANY_QT_DEVICE AND PLPLOT_USE_QT5)

# The above completes dealing with the external libraries.  Now
# deal with the internal libraries that are built by the PLplot
# build system.

if(WITH_CSA)
  list(APPEND libplplot_LINK_LIBRARIES PLPLOT::csirocsa)
  list(APPEND pc_libplplot_LINK_FLAGS -l${WRITEABLE_TARGET}csirocsa)
  include_directories(
    ${PROJECT_BINARY_DIR}/lib/csa
    )
endif(WITH_CSA)

if(PL_HAVE_QHULL)
  list(APPEND libplplot_LINK_LIBRARIES PLPLOT::csironn)
  # No *.pc files for lib subdirectory libraries so must add relevant bits
  # to the plplot.pc file instead.
  if(QHULL_RPATH)
    list(APPEND pc_libplplot_LINK_FLAGS -l${WRITEABLE_TARGET}csironn -L${QHULL_RPATH} -lqhull)
  else(QHULL_RPATH)
    list(APPEND pc_libplplot_LINK_FLAGS -l${WRITEABLE_TARGET}csironn -lqhull)
  endif(QHULL_RPATH)

  # Needed by plgridd.c.
  include_directories(
    ${PROJECT_BINARY_DIR}/lib/nn
    )
endif(PL_HAVE_QHULL)

if(NOT ENABLE_DYNDRIVERS AND PLD_cgm)
  list(APPEND libplplot_LINK_LIBRARIES PLPLOT::nistcd)
  list(APPEND pc_libplplot_LINK_FLAGS -l${WRITEABLE_TARGET}nistcd)
  include_directories(
    ${PROJECT_SOURCE_DIR}/lib/nistcd
    ${PROJECT_BINARY_DIR}/lib/nistcd
    )
endif(NOT ENABLE_DYNDRIVERS AND PLD_cgm)

list(APPEND libplplot_LINK_LIBRARIES PLPLOT::qsastime)
list(APPEND pc_libplplot_LINK_FLAGS -l${WRITEABLE_TARGET}qsastime)

if(NOT ENABLE_DYNDRIVERS AND PLD_wxwidgets AND RT_LIB)
  list(APPEND libplplot_LINK_LIBRARIES ${RT_LIB})
  list(APPEND pc_libplplot_LINK_FLAGS ${RT_LIB})
endif(NOT ENABLE_DYNDRIVERS AND PLD_wxwidgets AND RT_LIB)

#message(STATUS "libplplot_LINK_LIBRARIES = ${libplplot_LINK_LIBRARIES}")

if(NOT ENABLE_DYNDRIVERS AND ANY_QT_DEVICE AND PLPLOT_USE_QT5)
  list(APPEND libplplot_LINK_LIBRARIES Qt5::Svg Qt5::Gui Qt5::PrintSupport)
endif(NOT ENABLE_DYNDRIVERS AND ANY_QT_DEVICE AND PLPLOT_USE_QT5)

configure_library_build(plplot "" "${plplot_LIB_SRCS}" "${libplplot_LINK_LIBRARIES}" "${LIB_INSTALL_RPATH}")
add_dependencies(${WRITEABLE_TARGET}plplot plhershey-unicode.h_built)

if(NOT ENABLE_DYNDRIVERS AND (PLD_wxwidgets OR PLD_wxpng))
  set_property(TARGET ${WRITEABLE_TARGET}plplot
    APPEND PROPERTY
    INCLUDE_DIRECTORIES ${PROJECT_BINARY_DIR}/bindings/wxwidgets
    )
endif(NOT ENABLE_DYNDRIVERS AND (PLD_wxwidgets OR PLD_wxpng))

if(NOT ENABLE_DYNDRIVERS AND ANY_QT_DEVICE)
  if(NOT PLPLOT_USE_QT5)
    # Update the target COMPILE_DEFINITIONS and INCLUDE_DIRECTORIES
    set_qt4_target_properties(${WRITEABLE_TARGET}plplot)
  endif(NOT PLPLOT_USE_QT5)

  set_target_properties(${WRITEABLE_TARGET}plplot PROPERTIES
    AUTOMOC ON
    AUTOMOC_MOC_OPTIONS "${PLPLOT_AUTOMOC_MOC_OPTIONS}"
    )
    # This is not required for g++ and many other C++ compilers, but
    # still this is best practice for AUTOMOC just in case there is a
    # compiler out there (which is allowed by the lax C/C++ standards)
    # that does not look in the source directory for the quoted form
    # of #included headers in the automoc-generated source code
    # ${CMAKE_CURRENT_BINARY_DIR}/<targetname>_automoc.cpp that is
    # specifically added by automoc to the list of source files for
    # the target.
    target_include_directories(${WRITEABLE_TARGET}plplot PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif(NOT ENABLE_DYNDRIVERS AND ANY_QT_DEVICE)

message("pc_libplplot_LINK_FLAGS = ${pc_libplplot_LINK_FLAGS}")

string(REGEX REPLACE ";" " "
  pc_libplplot_COMPILE_FLAGS
  "${pc_libplplot_COMPILE_FLAGS}"
  )
#message("pc_libplplot_COMPILE_FLAGS = ${pc_libplplot_COMPILE_FLAGS}")

# Configure pkg-config *.pc file corresponding to libplplot
pkg_config_file("c" "" " Core C library" "plplot" "${pc_libplplot_COMPILE_FLAGS}" "${pc_libplplot_LINK_FLAGS};${cxx_compiler_library_pathname_list}")
