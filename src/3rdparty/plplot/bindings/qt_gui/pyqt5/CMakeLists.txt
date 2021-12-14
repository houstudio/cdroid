# bindings/qt_gui/pyqt5/CMakeLists.txt
# Copyright (C) 2016 Hazen Babcock
# Copyright (C) 2009-2019 Alan W. Irwin
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

if(ENABLE_pyqt5)

  set(plplot_pyqt5_HDR ${CMAKE_CURRENT_BINARY_DIR}/sipAPIplplot_pyqt5.h)

  set_source_files_properties(
    ${plplot_pyqt5_HDR}
    PROPERTIES
    GENERATED ON
    )

  set(plplot_pyqt5_SRC
    ${CMAKE_CURRENT_BINARY_DIR}/sipplplot_pyqt5cmodule.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipplplot_pyqt5QtExtWidget.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipplplot_pyqt5QtPLDriver.cpp
    ${CMAKE_CURRENT_BINARY_DIR}/sipplplot_pyqt5QtPLWidget.cpp
    )
  set_source_files_properties(
    ${plplot_pyqt5_SRC}
    PROPERTIES
    GENERATED ON
    )

  #message("DEBUG: PYQT_SIP_DIR = ${PYQT_SIP_DIR}")
  #message("DEBUG: PYQT_SIP_FLAGS = ${PYQT_SIP_FLAGS}")

  add_custom_command(
    OUTPUT ${plplot_pyqt5_HDR} ${plplot_pyqt5_SRC}
    COMMAND ${SIP_EXECUTABLE} -c . -b plplot_pyqt5.sbf -I${PYQT_SIP_DIR} ${PYQT_SIP_FLAGS} ${CMAKE_CURRENT_SOURCE_DIR}/plplot_pyqt5.sip
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/plplot_pyqt5.sip
    )
  add_custom_target(generate_pyqt5_source
    DEPENDS ${plplot_pyqt5_HDR} ${plplot_pyqt5_SRC}
    )

  # Assume sip.h can be found in PYTHON_INCLUDE_PATH
  include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/qsastime
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/lib/qsastime
    ${PYTHON_INCLUDE_PATH}
    )

  if(ENABLE_DYNDRIVERS)
    set(pyqt5_tll_list
      PLPLOT::plplotqt
      PLPLOT::plplot
      ${PYTHON_LIBRARIES}
      Qt5::Svg Qt5::Gui Qt5::PrintSupport
      )
  else(ENABLE_DYNDRIVERS)
    # With static device drivers, all the plplotqt code is
    # in the plplot library so plplotqt becomes redundant.
    set(pyqt5_tll_list
      PLPLOT::plplot
      ${PYTHON_LIBRARIES}
      Qt5::Svg Qt5::Gui Qt5::PrintSupport
      )
  endif(ENABLE_DYNDRIVERS)
  # LIB_INSTALL_RPATH propagated from parent directory.
  configure_library_build(plplot_pyqt5 MODULE "${plplot_pyqt5_SRC}" "${pyqt5_tll_list}" "${LIB_INSTALL_RPATH}" ${PYTHON_INSTDIR})

  if(WRITEABLE_TARGET)
    # For this experimental configuration copy the generated
    # ${WRITEABLE_TARGET}plplot_pyqt5.so file to the plplot_pyqt5.so
    # name required by examples/python/pyqt5_example.py
    add_custom_command(TARGET ${WRITEABLE_TARGET}plplot_pyqt5 POST_BUILD
      COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_BINARY_DIR}/${WRITEABLE_TARGET}plplot_pyqt5.so ${CMAKE_CURRENT_BINARY_DIR}/plplot_pyqt5.so
      VERBATIM
      )
  endif(WRITEABLE_TARGET)

  if(NOT ${PYTHON_VERSION} VERSION_LESS "3.0.0" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    # The combination of python3 (at least for version 3.4) and sip
    # creates a useless shared object with no externally accessible
    # symbols when g++ with the -fvisibility=hidden option is used to
    # compile the module. As a result, python3 cannot load this
    # module.  Work around this issue by appending
    # -fvisibility=default to the compiler options for this target for
    # all python 3 cases (although it is not clear if this bug has
    # been fixed for python 3 versions higher than 3.4). This
    # visibility bug does not occur for Python 2.
    target_compile_options(${WRITEABLE_TARGET}plplot_pyqt5 PRIVATE "-fvisibility=default")
  endif(NOT ${PYTHON_VERSION} VERSION_LESS "3.0.0" AND ${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")

  if(WIN32_OR_CYGWIN)
    set_target_properties(
      ${WRITEABLE_TARGET}plplot_pyqt5
      PROPERTIES
      PREFIX ""
      SUFFIX ".pyd"
      )
  else(WIN32_OR_CYGWIN)
    set_target_properties(${WRITEABLE_TARGET}plplot_pyqt5 PROPERTIES PREFIX "")
  endif(WIN32_OR_CYGWIN)

  add_dependencies(${WRITEABLE_TARGET}plplot_pyqt5 generate_pyqt5_source)

endif(ENABLE_pyqt5)
