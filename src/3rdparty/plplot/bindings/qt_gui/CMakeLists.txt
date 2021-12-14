# bindings/qt_gui/CMakeLists.txt
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

if(ENABLE_qt)
  if(ANY_QT_DEVICE)
    # Set up Qt4- or Qt5-based build environment.
    set(qt_SOURCE plqt.cpp)
    include_directories(
      ${CMAKE_SOURCE_DIR}/include
      ${CMAKE_SOURCE_DIR}/lib/qsastime
      ${CMAKE_SOURCE_DIR}/lib/nistcd
      ${CMAKE_SOURCE_DIR}/drivers
      ${CMAKE_BINARY_DIR}
      ${CMAKE_BINARY_DIR}/include
      ${CMAKE_BINARY_DIR}/lib/qsastime
      ${CMAKE_BINARY_DIR}/lib/nistcd
      )

    # Must add qt.h to explicit sources for AUTOMOC method to work for either
    # Qt4 or Qt5.
    list(APPEND qt_SOURCE ${CMAKE_SOURCE_DIR}/include/qt.h)

    if(PLPLOT_USE_QT5)
      # According to advice from Steve Kelly on the Cmake list, the Qt5
      # Gui component is a dependency of the Qt5 Svg component so is not
      # needed here, but I will leave it in since it is informative.
      set(libplplotqt_LINK_LIBRARIES PLPLOT::plplot ${MATH_LIB} Qt5::Svg Qt5::Gui Qt5::PrintSupport)
    else(PLPLOT_USE_QT5)
      set(libplplotqt_LINK_LIBRARIES PLPLOT::plplot ${MATH_LIB} )
    endif(PLPLOT_USE_QT5)

    # qt_RPATH set in cmake/modules/qt.cmake. This is only used for the
    # install-tree rpath since cmake handles the build-tree rpath
    # automatically (so long as full pathnames to libraries are used).
    #message("qt_RPATH = ${qt_RPATH}")
    set(LIB_INSTALL_RPATH
      # plplotqt depends on the plplot library.
      ${LIB_DIR}
      # plplotqt depends on the Qt libraries.
      ${qt_RPATH}
      )
      #message("LIB_INSTALL_RPATH = ${LIB_INSTALL_RPATH}")
    configure_library_build(plplotqt "" "${qt_SOURCE}" "${libplplotqt_LINK_LIBRARIES}" "${LIB_INSTALL_RPATH}")

    if(NOT PLPLOT_USE_QT5)
      if(NOT QT_LIBRARIES)
	message(FATAL_ERROR "PLplot build-system inconsistency in bindings/qt_gui subdirectory.")
      endif(NOT QT_LIBRARIES)
      # According to the latter part of a discussion on plplot-devel in
      # 2013 with the subject line "Can't build on OS X Lion", Andrew
      # and Jerry found together (using the old LINK_INTERFACE_LIBRARIES
      # approach) that is was necessary that ${QT_LIBRARIES} was always
      # transitively linked.  I think this is covered in the recent
      # documentation of the PUBLIC keyword where it is stated that "If
      # a dependency is additionally used in the header files of a
      # library (e.g. for class inheritance), then it should be
      # specified as a ``PUBLIC`` dependency."
      # Therefore, my conclusion is that the PUBLIC keyword should
      # be used here in all cases, but further checking of this assumption
      # (by looking for overlinking issues) is desireable.
      target_link_libraries(${WRITEABLE_TARGET}plplotqt PUBLIC ${QT_LIBRARIES})
      # Update the target COMPILE_DEFINITIONS and INCLUDE_DIRECTORIES
      set_qt4_target_properties(${WRITEABLE_TARGET}plplotqt)
    endif(NOT PLPLOT_USE_QT5)

    set_target_properties(${WRITEABLE_TARGET}plplotqt PROPERTIES
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
    target_include_directories(${WRITEABLE_TARGET}plplotqt PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

    # Configure pkg-config *.pc file corresponding to libplplotqt
    pkg_config_file("qt" "Qt" " Qt binding" "plplotqt" "${pc_qt_COMPILE_FLAGS}" "-lplplot;${pc_qt_LIBRARIES_LIST};${MATH_LIB}")

    add_subdirectory(pyqt4)
    add_subdirectory(pyqt5)
    add_subdirectory(smoke)
  else(ANY_QT_DEVICE)
    message(FATAL_ERROR "Internal build system inconsistency between ENABLE_qt true and ANY_QT_DEVICE false.")
  endif(ANY_QT_DEVICE)
endif(ENABLE_qt)
