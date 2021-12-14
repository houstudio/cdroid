# bindings/wxwidgets/CMakeLists.txt
# Copyright (C) 2007 Werner Smekal
# Copyright (C) 2015-2019 Alan W. Irwin
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

# ENABLE_wxwidgets is only true if ENABLE_cxx is also true.
if(ENABLE_wxwidgets)
  if(OLD_WXWIDGETS)
    set(plplotwxwidgets_LIB_SRCS
      deprecated_wxPLplotstream.cpp
      deprecated_wxPLplotwindow.cpp)
    # configure header file - we need to make sure, that the user of the bindings
    # can't set the wrong options (use freetype/agg)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/deprecated_wxPLplotstream.h.in
      ${CMAKE_CURRENT_BINARY_DIR}/deprecated_wxPLplotstream.h
      )
    set(plplotwxwidgets_INSTALLED_HEADERS
      ${CMAKE_CURRENT_BINARY_DIR}/deprecated_wxPLplotstream.h
      deprecated_wxPLplotwindow.h
      )
  else(OLD_WXWIDGETS)
    configure_file(
      ${CMAKE_CURRENT_SOURCE_DIR}/wxPLplot_nanosec.h.in
      ${CMAKE_CURRENT_BINARY_DIR}/wxPLplot_nanosec.h
      )
    set(plplotwxwidgets_LIB_SRCS
      wxPLplotstream.cpp
      )
    if(PLPLOT_WX_DEBUG_OUTPUT AND PLPLOT_WX_NANOSEC)
      list(APPEND plplotwxwidgets_LIB_SRCS wxPLplot_nanosec.cpp)
    endif(PLPLOT_WX_DEBUG_OUTPUT AND PLPLOT_WX_NANOSEC)

    set(plplotwxwidgets_INSTALLED_HEADERS
      wxPLplotstream.h
      wxPLplotwindow.h
      ${CMAKE_CURRENT_BINARY_DIR}/wxPLplot_nanosec.h
      )
  endif(OLD_WXWIDGETS)
  # Set the include path
  include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/qsastime
    ${CMAKE_SOURCE_DIR}/bindings/c++
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/lib/qsastime
    ${CMAKE_CURRENT_BINARY_DIR}
    )

  set_source_files_properties(${plplotwxwidgets_LIB_SRCS}
    PROPERTIES COMPILE_FLAGS "${wxwidgets_COMPILE_FLAGS}"
    )

  # plplotwxwidgets depends on the plplot library.
  set(LIB_INSTALL_RPATH ${LIB_DIR})
  # plplotwxwidgets depends on the wxwidgets libraries.
  list(APPEND LIB_INSTALL_RPATH ${wxwidgets_RPATH})

  configure_library_build(plplotwxwidgets "" "${plplotwxwidgets_LIB_SRCS}" "PLPLOT::plplot;PLPLOT::plplotcxx;${wxwidgets_LINK_FLAGS}" "${LIB_INSTALL_RPATH}")

  INCLUDE_DIRECTORIES( ${wxWidgets_INCLUDE_DIRS} )
  IF(wxWidgets_DEFINITIONS)
    set_property(TARGET ${WRITEABLE_TARGET}plplotwxwidgets APPEND
      PROPERTY COMPILE_DEFINITIONS ${wxWidgets_DEFINITIONS})
  ENDIF(wxWidgets_DEFINITIONS)

  IF(wxWidgets_DEFINITIONS_DEBUG)
    set_property(TARGET plplotwxwidgets APPEND
      PROPERTY COMPILE_DEFINITIONS_DEBUG ${wxWidgets_DEFINITIONS_DEBUG})
  ENDIF(wxWidgets_DEFINITIONS_DEBUG)

  install(FILES
    ${plplotwxwidgets_INSTALLED_HEADERS}
    DESTINATION ${INCLUDE_DIR}
    )

  # Determine libplplotwxwidgets_COMPILE_FLAGS
  set(wxWidgets_DEFINITIONS_all "${wxWidgets_DEFINITIONS}")
  if(wxWidgets_DEFINITIONS_DEBUG)
    set(wxWidgets_DEFINITIONS_all "${wxWidgets_DEFINITIONS};${wxWidgets_DEFINITIONS_DEBUG}")
  endif(wxWidgets_DEFINITIONS_DEBUG)

  STRING(REGEX REPLACE ";" " -D"
    libplplotwxwidgets_COMPILE_FLAGS
    "-D${wxWidgets_DEFINITIONS_all}"
    )
  STRING(REGEX REPLACE ";" " -I"
    wxWidgets_IFLAGS
    "-I${wxWidgets_INCLUDE_DIRS}"
    )
  set(libplplotwxwidgets_COMPILE_FLAGS
    "${libplplotwxwidgets_COMPILE_FLAGS} ${wxWidgets_IFLAGS}"
    )

  # Configure pkg-config *.pc file corresponding to libplplotwxwidgets
  pkg_config_file("wxwidgets" "wxWidgets" " wxWidgets binding" "plplotwxwidgets" "${libplplotwxwidgets_COMPILE_FLAGS}" "-lplplot;-lplplotcxx;${wxwidgets_LINK_FLAGS}")

endif(ENABLE_wxwidgets)
