# bindings/lua/CMakeLists.txt
# Copyright (C) 2008 Werner Smekal
# Copyright (C) 2017-2019 Alan W. Irwin

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

if(ENABLE_lua)

  # This is currently the include list for swig.
  set(lua_interface_INCLUDE_PATHS
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/qsastime
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/lib/qsastime
    ${CMAKE_CURRENT_BINARY_DIR}
    ${LUA_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/bindings/swig-support
    )
  include_directories(${lua_interface_INCLUDE_PATHS})

  set(CMAKE_SWIG_FLAGS -DSWIG_LUA -module ${WRITEABLE_TARGET}plplotluac)
  set(CMAKE_SWIG_OUTDIR ${CMAKE_CURRENT_BINARY_DIR})

  set_source_files_properties(plplotluac.i
    PROPERTIES SWIG_MODULE_NAME plplotluac
    )

  set(SWIG_MODULE_plplotluac_EXTRA_DEPS
    ${CMAKE_SOURCE_DIR}/bindings/swig-support/plplotcapi.i)

  configure_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/lua_namespace.i.in
  ${CMAKE_CURRENT_BINARY_DIR}/lua_namespace.i
  @ONLY
  )

  # plplotluac depends on the plplot library.
  set(LIB_INSTALL_RPATH ${LIB_DIR})

  # Configure build of swig-generated C wrapper.
  configure_library_build(plplotluac lua plplotluac.i "PLPLOT::plplot;${LUA_LIBRARIES}" "${LIB_INSTALL_RPATH}" ${LUA_DIR})

endif(ENABLE_lua)
