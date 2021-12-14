# examples/c++/CMakeLists.txt
# Copyright (C) 2006 Andrew Ross
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

set(cxx_STRING_INDICES
  "01cc"
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

if(ENABLE_wxwidgets)
  set(wxPLplotDemo_SRCS ${wxdemo_name}.cpp)
endif(ENABLE_wxwidgets)

if(CORE_BUILD)
  set(cxx_SRCS plc++demos.h)
  foreach(STRING_INDEX ${cxx_STRING_INDICES})
    set(cxx_SRCS ${cxx_SRCS} x${STRING_INDEX}.cc)
  endforeach(STRING_INDEX ${cxx_STRING_INDICES})

  if(PLD_extqt)
    set(qt_INSTALLED_FILES qt_PlotWindow.cpp qt_PlotWindow.h qt_example.cpp README.qt_example)
  endif(PLD_extqt)

  install(FILES ${cxx_SRCS} ${wxPLplotDemo_SRCS} ${qt_INSTALLED_FILES} DESTINATION ${DATA_DIR}/examples/c++)

  set(CXX "${CMAKE_CXX_COMPILER} ${CMAKE_CXX_COMPILER_ARG1} ${CMAKE_CXX_FLAGS}")

  install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/CMakeLists.txt
    DESTINATION ${DATA_DIR}/examples/c++
    )
endif(CORE_BUILD)

if(BUILD_TEST)
  if(CORE_BUILD)
    include_directories(
      ${PROJECT_SOURCE_DIR}/bindings/c++
      ${PROJECT_SOURCE_DIR}/include
      ${PROJECT_BINARY_DIR}/include
      ${PROJECT_SOURCE_DIR}/lib/qsastime
      ${PROJECT_BINARY_DIR}/lib/qsastime
      )
  else(CORE_BUILD)
    include_directories(${INCLUDE_DIR})
  endif(CORE_BUILD)

  set_property(GLOBAL PROPERTY TARGETS_examples_cxx)
  foreach(STRING_INDEX ${cxx_STRING_INDICES})
    add_executable(x${STRING_INDEX} x${STRING_INDEX}.cc)
    if(BUILD_SHARED_LIBS)
      set_target_properties(x${STRING_INDEX} PROPERTIES
	COMPILE_DEFINITIONS "USINGDLL"
	)
    endif(BUILD_SHARED_LIBS)
    target_link_libraries(x${STRING_INDEX} PLPLOT::plplotcxx ${MATH_LIB})
    set_property(GLOBAL APPEND PROPERTY TARGETS_examples_cxx x${STRING_INDEX})
  endforeach(STRING_INDEX ${cxx_STRING_INDICES})
endif(BUILD_TEST)

if(ENABLE_wxwidgets)
  # Build wxwidgets applications with same wxwidgets compile and link flags
  # as used with the PLplot wxwidgets device driver.
  if(CORE_BUILD)
    include_directories(
      ${PROJECT_SOURCE_DIR}/bindings/wxwidgets
      ${PROJECT_BINARY_DIR}/bindings/wxwidgets
      )
  endif(CORE_BUILD)
  if(BUILD_TEST)
    set_source_files_properties(${wxPLplotDemo_SRCS}
      PROPERTIES COMPILE_FLAGS "${wxwidgets_COMPILE_FLAGS}")
    add_executable(wxPLplotDemo WIN32 ${wxPLplotDemo_SRCS})
    if(BUILD_SHARED_LIBS)
      set_target_properties(wxPLplotDemo PROPERTIES
	COMPILE_DEFINITIONS "USINGDLL"
	)
    endif(BUILD_SHARED_LIBS)
    target_link_libraries(wxPLplotDemo PLPLOT::plplotwxwidgets PLPLOT::plplotcxx ${wxwidgets_LINK_FLAGS} ${MATH_LIB})
  endif(BUILD_TEST)
  # Transform wxwidgets_LINK_FLAGS from list of libraries to the standard pkg-config form
  # for use in configuring Makefile.examples.
  pkg_config_link_flags(pkg_config_wxwidgets_LINK_FLAGS "${wxwidgets_LINK_FLAGS}")
endif(ENABLE_wxwidgets)

if(PLD_extqt)
  if(CORE_BUILD)
    set(BUILD_qt_example ON)
    if(NOT PLPLOT_USE_QT5 AND NOT QT_LIBRARIES)
      message(FATAL_ERROR "include(${QT_USE_FILE}) failed in core build examples/c++ subdirectory.")
    endif(NOT PLPLOT_USE_QT5 AND NOT QT_LIBRARIES)
  else(CORE_BUILD)
    # Find Qt components that we need to build qt_example
    if(PLPLOT_USE_QT5)
      # find_package(Qt5 ...) already done by find_package(plplot) for
      # the reasons given in src/plplotConfig.cmake.  So no need to repeat
      # here.
      # find_package(Qt5 5.3.1 COMPONENTS Svg Gui PrintSupport)
      # So only need to check those results here for this particular
      # use of Qt5.
      if(Qt5_FOUND AND Qt5_VERSION_MAJOR STREQUAL "${CORE_Qt5_VERSION_MAJOR}" AND Qt5_VERSION_MINOR STREQUAL "${CORE_Qt5_VERSION_MINOR}" AND Qt5_VERSION_PATCH STREQUAL "${CORE_Qt5_VERSION_PATCH}")
	set(BUILD_qt_example ON)
      else(Qt5_FOUND AND Qt5_VERSION_MAJOR STREQUAL "${CORE_Qt5_VERSION_MAJOR}" AND Qt5_VERSION_MINOR STREQUAL "${CORE_Qt5_VERSION_MINOR}" AND Qt5_VERSION_PATCH STREQUAL "${CORE_Qt5_VERSION_PATCH}")
	message(STATUS "CORE_Qt5_FOUND = ON")
	message(STATUS "CORE_Qt5_VERSION_MAJOR = ${CORE_Qt5_VERSION_MAJOR}")
	message(STATUS "CORE_Qt5_VERSION_MINOR = ${CORE_Qt5_VERSION_MINOR}")
	message(STATUS "CORE_Qt5_VERSION_PATCH = ${CORE_Qt5_VERSION_PATCH}")
	message(STATUS "Qt5_FOUND = ${Qt5_FOUND}")
	message(STATUS "Qt5_VERSION_MAJOR = ${Qt5_VERSION_MAJOR}")
	message(STATUS "Qt5_VERSION_MINOR = ${Qt5_VERSION_MINOR}")
	message(STATUS "Qt5_VERSION_PATCH = ${Qt5_VERSION_PATCH}")
	message(STATUS "WARNING: Qt5 core build-tree and install-tree inconsistency so qt_example will not be built")
	set(BUILD_qt_example OFF)
      endif(Qt5_FOUND AND Qt5_VERSION_MAJOR STREQUAL "${CORE_Qt5_VERSION_MAJOR}" AND Qt5_VERSION_MINOR STREQUAL "${CORE_Qt5_VERSION_MINOR}" AND Qt5_VERSION_PATCH STREQUAL "${CORE_Qt5_VERSION_PATCH}")
    else(PLPLOT_USE_QT5)
      # Find Qt4 all over again here rather than simply copying variables
      # from the core build to this installed examples build.
      find_package(Qt4 4.8.2 REQUIRED QtCore QtGui QtSvg)
      # To avoid Qt4 version mismatch make sure the Qt version matches that
      # used by the core build.
      if(Qt4_FOUND AND QT_VERSION_MAJOR STREQUAL "${CORE_QT_VERSION_MAJOR}" AND QT_VERSION_MINOR STREQUAL "${CORE_QT_VERSION_MINOR}" AND QT_VERSION_PATCH STREQUAL "${CORE_QT_VERSION_PATCH}")
	include(ndp_UseQt4)
	if(NOT QT_LIBRARIES)
          message(FATAL_ERROR "include(${QT_USE_FILE}) failed in installed examples build examples/c++ subdirectory.")
	endif(NOT QT_LIBRARIES)
	set(BUILD_qt_example ON)
      else(Qt4_FOUND AND QT_VERSION_MAJOR STREQUAL "${CORE_QT_VERSION_MAJOR}" AND QT_VERSION_MINOR STREQUAL "${CORE_QT_VERSION_MINOR}" AND QT_VERSION_PATCH STREQUAL "${CORE_QT_VERSION_PATCH}")
	message(STATUS "CORE_Qt4_FOUND = ON")
	message(STATUS "CORE_QT_VERSION_MAJOR = ${CORE_QT_VERSION_MAJOR}")
	message(STATUS "CORE_QT_VERSION_MINOR = ${CORE_QT_VERSION_MINOR}")
	message(STATUS "CORE_QT_VERSION_PATCH = ${CORE_QT_VERSION_PATCH}")
	message(STATUS "Qt4_FOUND = ${Qt4_FOUND}")
	message(STATUS "QT_VERSION_MAJOR = ${QT_VERSION_MAJOR}")
	message(STATUS "QT_VERSION_MINOR = ${QT_VERSION_MINOR}")
	message(STATUS "QT_VERSION_PATCH = ${QT_VERSION_PATCH}")
	message(STATUS "WARNING: Qt4 core build-tree and install-tree inconsistency so qt_example will not be built")
	set(BUILD_qt_example OFF)
      endif(Qt4_FOUND AND QT_VERSION_MAJOR STREQUAL "${CORE_QT_VERSION_MAJOR}" AND QT_VERSION_MINOR STREQUAL "${CORE_QT_VERSION_MINOR}" AND QT_VERSION_PATCH STREQUAL "${CORE_QT_VERSION_PATCH}")
    endif(PLPLOT_USE_QT5)
  endif(CORE_BUILD)
endif(PLD_extqt)

if(BUILD_TEST AND BUILD_qt_example)
  # For both Qt4 and Qt5 AUTOMOC method works fine to process
  # qt_PlotWindow.h in the same directory with the same base name as
  # qt_PlotWindow.cpp.  Do not add qt.h to explicit sources since
  # that is only relevant for a separate library (plplotqt or plplot)
  # that is linked.
  add_executable(qt_example qt_example.cpp qt_PlotWindow.cpp)
  if(BUILD_SHARED_LIBS)
    set_target_properties(qt_example PROPERTIES
      COMPILE_DEFINITIONS "USINGDLL"
      )
  endif(BUILD_SHARED_LIBS)

  if(PLPLOT_USE_QT5)
    if(ENABLE_DYNDRIVERS)
      target_link_libraries(qt_example PLPLOT::plplotqt PLPLOT::plplot Qt5::Svg Qt5::Gui Qt5::PrintSupport)
    else(ENABLE_DYNDRIVERS)
      # In this case plplot contains all the code in plplotqt so there is no
      # need to link to plplotqt at all, and in fact for some reason it
      # causes memory management issues to link to both plplot and plplotqt in this case.
      target_link_libraries(qt_example PLPLOT::plplot Qt5::Svg Qt5::Gui Qt5::PrintSupport)
    endif(ENABLE_DYNDRIVERS)
  else(PLPLOT_USE_QT5)
      if(ENABLE_DYNDRIVERS)
      target_link_libraries(qt_example ${QT_LIBRARIES} PLPLOT::plplotqt PLPLOT::plplot)
    else(ENABLE_DYNDRIVERS)
      # In this case plplot contains all the code in plplotqt so there is no
      # need to link to plplotqt at all, and in fact for some reason it
      # causes memory management issues to link to both plplot and plplotqt in this case.
      target_link_libraries(qt_example ${QT_LIBRARIES} PLPLOT::plplot)
    endif(ENABLE_DYNDRIVERS)
    # Update the target COMPILE_DEFINITIONS and INCLUDE_DIRECTORIES
    set_qt4_target_properties(qt_example)
  endif(PLPLOT_USE_QT5)
  set_target_properties(qt_example PROPERTIES
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
    target_include_directories(qt_example PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
endif(BUILD_TEST AND BUILD_qt_example)

if(CORE_BUILD)
  if(ENABLE_qt)
    # Set additional variables used to configure Makefile.examples.in
    # which are required for the traditional build of the qt_example.

    if(PLPLOT_USE_QT5)
      # Apparently, Qt5Core_EXECUTABLE_COMPILE_FLAGS (= -fPIE on Linux
      # systems) needs to be a compiler option to build executables such
      # as qt_example in the traditional way for the Qt5 case.
      set(Qt_EXECUTABLE_COMPILE_FLAGS "${Qt5Core_EXECUTABLE_COMPILE_FLAGS}")
    else(PLPLOT_USE_QT5)
      # An equivalent variable is not defined for the Qt4 case because
      # apparently executables that are linked to Qt4 have no
      # equivalent special compile flag needs.
      set(Qt_EXECUTABLE_COMPILE_FLAGS)
    endif(PLPLOT_USE_QT5)

    # Need link flags in pkg-config form for traditional builds of
    # qt_example where that example refers to Qt functions so needs to
    # be directly linked to Qt libraries.  Note, pc_qt_LIBRARIES_LIST is
    # defined for both Qt4 and Qt5.
    pkg_config_link_flags(pc_qt_link_flags "${pc_qt_LIBRARIES_LIST}")
    message(STATUS "pc_qt_link_flags = ${pc_qt_link_flags}")
  endif(ENABLE_qt)
  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Makefile.examples.in
    ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    )

  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/Makefile.examples
    DESTINATION ${DATA_DIR}/examples/c++
    RENAME Makefile
    )

endif(CORE_BUILD)
