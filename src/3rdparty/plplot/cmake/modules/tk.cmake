# cmake/modules/tk.cmake
#
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
# along with the file PLplot; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA

# Module for determining all configuration variables related to the Tk
# related device drivers (tk, ntk, tkwin).

# The following variables are set/modified for the tk device:
# PLD_tk		  - ON means the tk device is enabled.
# tk_COMPILE_FLAGS	  - individual COMPILE_FLAGS required to compile
# 			    the tk device.
# tk_LINK_FLAGS	  	  - individual LINK_FLAGS for the dynamic tk device.
# tk_RPATH                - rpath for tk device driver.
# DRIVERS_LINK_FLAGS	  - list of LINK_FLAGS for all static devices.
# tk_SOURCE		  - list of source files other than tk.c

# The following variables are set/modified for the ntk device:
# PLD_ntk		  - ON means the ntk device is enabled.
# ntk_COMPILE_FLAGS	  - individual COMPILE_FLAGS required to compile
# 			    the ntk device.
# ntk_LINK_FLAGS	  - individual LINK_FLAGS for the dynamic ntk device.
# ntk_RPATH               - rpath for ntk device driver.
# DRIVERS_LINK_FLAGS	  - list of LINK_FLAGS for all static devices.

# The following variables are set/modified for the tkwin device:
# PLD_tkwin		  - ON means the tkwin device is enabled.
# tkwin_COMPILE_FLAGS	  - individual COMPILE_FLAGS required to compile
# 			    the tkwin device.
# tkwin_LINK_FLAGS	  - individual LINK_FLAGS for the dynamic tkwin device.
# tkwin_RPATH             - rpath for tkwin device driver.
# DRIVERS_LINK_FLAGS	  - list of LINK_FLAGS for all static devices.
# tkwin_SOURCE		  - list of source files other than tkwin.c

if(NOT ENABLE_tk AND (PLD_tk OR PLD_ntk OR PLD_tkwin))
  message(STATUS
    "WARNING: ENABLE_tk OFF.  Therefore turning off tk, tkwin, and ntk devices"
    )
  set(PLD_tk OFF CACHE BOOL "Enable tk device" FORCE)
  set(PLD_ntk OFF CACHE BOOL "Enable ntk device" FORCE)
  set(PLD_tkwin OFF CACHE BOOL "Enable tkwin device" FORCE)
endif(NOT ENABLE_tk AND (PLD_tk OR PLD_ntk OR PLD_tkwin))

if(NOT X11_FOUND AND (PLD_tk OR PLD_tkwin))
  message(STATUS "WARNING: X11 not found.  Therefore turning off tk and tkwin devices that depend on it")
  set(PLD_tk OFF CACHE BOOL "Enable Tk device" FORCE)
  set(PLD_tkwin OFF CACHE BOOL "Enable Tk device" FORCE)
endif(NOT X11_FOUND AND (PLD_tk OR PLD_tkwin))

if(PLD_tk OR PLD_ntk OR PLD_tkwin)
  # Transform TK_INCLUDE_PATH (which is a list) to blank-delimited flag form.
  string(REGEX REPLACE ";" "\" -I\"" TKLIB_COMPILE_FLAGS "-I\"${TK_INCLUDE_PATH}\"")
  message(STATUS "TKLIB_COMPILE_FLAGS = ${TKLIB_COMPILE_FLAGS}")
endif(PLD_tk OR PLD_ntk OR PLD_tkwin)

if(PLD_tk)
  set(tk_COMPILE_FLAGS
  "-I\"${TCL_INCLUDE_PATH}\" ${TKLIB_COMPILE_FLAGS} -I\"${PROJECT_SOURCE_DIR}\"/bindings/tcl -I\"${PROJECT_BINARY_DIR}\"/bindings/tcl -I\"${PROJECT_SOURCE_DIR}\"/bindings/tk"
  )
  set(tk_LINK_FLAGS PLPLOT::plplottcltk ${TCL_LIBRARY} ${TK_LIBRARY})
  set(tk_RPATH ${TCL_TK_RPATH})
  set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${TCL_LIBRARY} ${TK_LIBRARY})
  if(NOT ENABLE_DYNDRIVERS)
    # All source that is in libplplottcltk
    set(
    tk_SOURCE
    ${PROJECT_SOURCE_DIR}/bindings/tcl/tclAPI.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/Pltk_Init.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/plframe.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/plr.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/tcpip.c
    )
    # All source that is in libtclmatrix
    set(
    tk_SOURCE
    ${tk_SOURCE}
    ${PROJECT_SOURCE_DIR}/bindings/tcl/tclMatrix.c
    ${PROJECT_SOURCE_DIR}/bindings/tcl/matrixInit.c
    )
    if(ENABLE_itcl)
	  #note tk_compile flags already has its quote marks
      set(tk_COMPILE_FLAGS
      "${tk_COMPILE_FLAGS} -I\"${ITCL_INCLUDE_PATH}\""
      )
      set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${ITCL_LIBRARY})
    endif(ENABLE_itcl)
    if(ENABLE_itk)
      set(tk_COMPILE_FLAGS
      "${tk_COMPILE_FLAGS} -I\"${ITK_INCLUDE_PATH}\""
      )
      set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${ITK_LIBRARY})
    endif(ENABLE_itk)
  endif(NOT ENABLE_DYNDRIVERS)
  message(STATUS "tk_COMPILE_FLAGS = ${tk_COMPILE_FLAGS}")
  message(STATUS "tk_LINK_FLAGS = ${tk_LINK_FLAGS}")
  message(STATUS "tk_RPATH = ${tk_RPATH}")
endif(PLD_tk)

if(PLD_ntk)
  #note TKLIB_COMPILE_FLAGS already has its quote marks
  set(ntk_COMPILE_FLAGS "-I\"${TCL_INCLUDE_PATH}\" ${TKLIB_COMPILE_FLAGS}")
  set(ntk_LINK_FLAGS ${TCL_LIBRARY} ${TK_LIBRARY})
  set(ntk_RPATH ${TCL_TK_RPATH})
  set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${ntk_LINK_FLAGS})
  message(STATUS "ntk_COMPILE_FLAGS = ${ntk_COMPILE_FLAGS}")
  message(STATUS "ntk_LINK_FLAGS = ${ntk_LINK_FLAGS}")
  message(STATUS "ntk_RPATH = ${ntk_RPATH}")
endif(PLD_ntk)

if(PLD_tkwin)
  set(tkwin_COMPILE_FLAGS
    "-I\"${TCL_INCLUDE_PATH}\" ${TKLIB_COMPILE_FLAGS} -I\"${PROJECT_SOURCE_DIR}\"/bindings/tcl -I\"${PROJECT_BINARY_DIR}\"/bindings/tcl -I\"${PROJECT_SOURCE_DIR}\"/bindings/tk-x-plat -I\"${PROJECT_SOURCE_DIR}\"/bindings/tk"
    )

  if(USE_TCL_TK_STUBS)
    set(tkwin_LINK_FLAGS PLPLOT::plplottcltk ${TCL_STUB_LIBRARY} ${TK_STUB_LIBRARY} ${X11_LIBRARIES})
    # tkwin_RPATH should be undefined for this case since stubs versions
    # of the libraries are static (in my experience so far).
  else(USE_TCL_TK_STUBS)
    set(tkwin_LINK_FLAGS PLPLOT::plplottcltk ${TCL_LIBRARY} ${TK_LIBRARY} ${X11_LIBRARIES})
    set(tkwin_RPATH ${TCL_TK_RPATH})
  endif(USE_TCL_TK_STUBS)

  set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${TCL_LIBRARY} ${TK_LIBRARY})
  message(STATUS "tkwin_COMPILE_FLAGS = ${tkwin_COMPILE_FLAGS}")
  message(STATUS "tkwin_LINK_FLAGS = ${tkwin_LINK_FLAGS}")
  message(STATUS "tkwin_RPATH = ${tkwin_RPATH}")
  set(
  tkwin_SOURCE
  ${PROJECT_SOURCE_DIR}/bindings/tk-x-plat/Plplotter_Init.c
  ${PROJECT_SOURCE_DIR}/bindings/tk-x-plat/plplotter.c
  )
  if(NOT ENABLE_DYNDRIVERS AND NOT PLD_tk)
    # All source that is in libplplottcltk
    set(
    tkwin_SOURCE
    ${tkwin_SOURCE}
    ${PROJECT_SOURCE_DIR}/bindings/tcl/tclAPI.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/Pltk_Init.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/plframe.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/plr.c
    ${PROJECT_SOURCE_DIR}/bindings/tk/tcpip.c
    )
    # All source that is in libtclmatrix
    set(
    tkwin_SOURCE
    ${tkwin_SOURCE}
    ${PROJECT_SOURCE_DIR}/bindings/tcl/tclMatrix.c
    ${PROJECT_SOURCE_DIR}/bindings/tcl/matrixInit.c
    )
    if(ENABLE_itcl)
      set(tkwin_COMPILE_FLAGS
      "${tkwin_COMPILE_FLAGS} -I${ITCL_INCLUDE_PATH}"
      )
      set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${ITCL_LIBRARY})
    endif(ENABLE_itcl)
    if(ENABLE_itk)
      set(tkwin_COMPILE_FLAGS
      "${tkwin_COMPILE_FLAGS} -I${ITK_INCLUDE_PATH}"
      )
      set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${ITK_LIBRARY})
    endif(ENABLE_itk)
  endif(NOT ENABLE_DYNDRIVERS AND NOT PLD_tk)
endif(PLD_tkwin)

if(NOT ENABLE_DYNDRIVERS AND (PLD_tk OR PLD_tkwin))
  set(tcltk_in_plplot_library ON)
  set(tcltk_in_plplot_library_true "")
  set(tcltk_in_plplot_library_false "#")
else(NOT ENABLE_DYNDRIVERS AND (PLD_tk OR PLD_tkwin))
  set(tcltk_in_plplot_library OFF)
  set(tcltk_in_plplot_library_true "#")
  set(tcltk_in_plplot_library_false "")
endif(NOT ENABLE_DYNDRIVERS AND (PLD_tk OR PLD_tkwin))

