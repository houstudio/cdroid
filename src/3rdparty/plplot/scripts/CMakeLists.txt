# scripts/CMakeLists.txt for PLplot
###
### Process this file with cmake to produce Makefile
###
# Copyright (C) 2006 Alan W. Irwin
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

if(HAVE_BASH)
  set(scripts_SCRIPTS "")
  if(PLD_plmeta)
    set(scripts_SCRIPTS ${scripts_SCRIPTS}
    plm2gif
    plpr
    )
  endif(PLD_plmeta)
  if(PLD_pstex)
    set(scripts_SCRIPTS ${scripts_SCRIPTS} pstex2eps)
  endif(PLD_pstex)
  if (NOT "${scripts_SCRIPTS}" STREQUAL "")
    install(PROGRAMS ${scripts_SCRIPTS} DESTINATION ${BIN_DIR})
  endif (NOT "${scripts_SCRIPTS}" STREQUAL "")
endif(HAVE_BASH)

if(ENABLE_tcl OR ENABLE_tk)
  install(PROGRAMS mktclIndex DESTINATION ${DATA_DIR}/examples/tcl)
endif(ENABLE_tcl OR ENABLE_tk)

