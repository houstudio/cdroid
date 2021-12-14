# bindings/tk-x-plat/CMakeLists.txt
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

if(PLD_tkwin)

  if(USE_DLL_SUBDIRECTORY)
    set(LIB_INSTALL_DIR ${BIN_DIR})
  else(USE_DLL_SUBDIRECTORY)
    set(LIB_INSTALL_DIR ${DRV_DIR})
  endif(USE_DLL_SUBDIRECTORY)

  if(tcltk_in_plplot_library)
    set(DLNAME_LIBRARY_DIR $<TARGET_FILE_DIR:PLPLOT::plplot>)
    set(DLNAME_LIBRARY $<TARGET_FILE_NAME:PLPLOT::plplot>)
  else(tcltk_in_plplot_library)
    set(DLNAME_LIBRARY_DIR $<TARGET_FILE_DIR:PLPLOT::tkwin>)
    set(DLNAME_LIBRARY $<TARGET_FILE_NAME:PLPLOT::tkwin>)
  endif(tcltk_in_plplot_library)

  # N.B. Results only available at generate time.
  configure_file_generate(
    ${CMAKE_CURRENT_SOURCE_DIR}/pkgIndex.tcl.in
    ${CMAKE_CURRENT_BINARY_DIR}/pkgIndex.tcl
    @ONLY
    )

endif(PLD_tkwin)
