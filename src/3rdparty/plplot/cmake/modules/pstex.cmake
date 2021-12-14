# cmake/modules/pstex.cmake
#
# Copyright (C) 2007  Alan W. Irwin
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
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Module for configuring the pstex device driver.

# The following variables are set/modified for the pstex device:
# PLD_pstex                  - ON means the pstex device is enabled.
# pstex_SOURCE               - list of source files other than pstex.c

# N.B. this device driver is parasitic on the ps device driver so you cannot
# enable pstex without ps being enabled.
if(PLD_pstex AND NOT (PLD_ps AND PLD_psc))
  message(STATUS
    "WARNING: PLD_ps and PLD_psc are not both ON so must set PLD_pstex (which depends on both of them) to OFF."
    )
  set(PLD_pstex OFF CACHE BOOL "Enable pstex device" FORCE)
endif(PLD_pstex AND NOT (PLD_ps AND PLD_psc))

if(PLD_pstex AND ENABLE_DYNDRIVERS)
  # N.B. the pstex.c code is parasitic on the ps.c code so must be combined
  # with the latter in the plug-in for the ENABLE_DYNDRIVERS case.
  set(pstex_SOURCE ${CMAKE_SOURCE_DIR}/drivers/ps.c)
endif(PLD_pstex AND ENABLE_DYNDRIVERS)
