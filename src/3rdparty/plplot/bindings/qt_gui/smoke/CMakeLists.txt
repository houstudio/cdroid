# bindings/qt_gui/smoke/CMakeLists.txt
# Copyright (C) 2014 Hazen Babcock
# Copyright (C) 2016-2019 Alan W. Irwin
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

if(ENABLE_smoke)

  configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/plplotqt.pro.in
    ${CMAKE_CURRENT_BINARY_DIR}/plplotqt.pro
    )

  set(plplot_smoke_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/smokedata.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/x_1.cpp
    )

  set_source_files_properties(
    ${plplot_smoke_SRC}
    PROPERTIES
    GENERATED ON
    )

  set(include_directories_LIST
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/qsastime
    ${CMAKE_SOURCE_DIR}/lib/nistcd
    ${CMAKE_SOURCE_DIR}/drivers
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/lib/qsastime
    ${CMAKE_BINARY_DIR}/lib/nistcd
    )

  include_directories(${include_directories_LIST})

  set(smokegen_I_options)
  foreach(directory ${include_directories_LIST})
    list(APPEND smokegen_I_options -I "${directory}")
  endforeach(directory ${include_directories_LIST})

  add_custom_command(
    OUTPUT ${plplot_smoke_SRC}
    COMMAND ${SMOKE_GEN_BIN}
    -config "${SMOKE_GEN_SHARED}/qt-config.xml" -smokeconfig "${CMAKE_CURRENT_SOURCE_DIR}/smokeconfig.xml" ${smokegen_I_options} -- ${CMAKE_SOURCE_DIR}/qt.h
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/smokeconfig.xml ${CMAKE_SOURCE_DIR}/include/qt.h
    VERBATIM
    )
  add_custom_target(generate_smoke_source
    DEPENDS ${plplot_smoke_SRC}
    )

  # N.B. We use Qt4 methods alone here because smoke
  # not implemented for Qt5.  See further comments
  # in cmake/modules/qt.cmake where smoke is automatically
  # disabled for the Qt5 case.

  # N.B. Apparently (from clean nm --undefined-only --demangle
  # results) there is no need to set up automoc here for qt.h, and in
  # fact when I did that it generated extra undefined symbols which
  # then were resolved by the symbols in libplplotqt.  But this
  # occurred for a build without -fvisibility=hidden, i.e., those
  # libplplotqt symbols may normally not be exported so linking might
  # not work at all for that case.  So until I understand more of what
  # is going on, I will ignore automoc here.
  set(smokeplplotqt_tll_list
    PLPLOT::plplotqt
    PLPLOT::plplot
    ${SMOKE_QTCORE_LIBRARY}
    ${SMOKE_QTGUI_LIBRARY}
    ${QT_LIBRARIES}
    )

  # LIB_INSTALL_RPATH propagated from parent directory.
  configure_library_build(smokeplplotqt "" "${plplot_smoke_SRC}" "${smokeplplotqt_tll_list}" "${LIB_INSTALL_RPATH}")
  # Update the target COMPILE_DEFINITIONS and INCLUDE_DIRECTORIES
  set_qt4_target_properties(smokeplplotqt)

  add_dependencies(smokeplplotqt generate_smoke_source)

endif(ENABLE_smoke)
