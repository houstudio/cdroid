# cmake/modules/python.cmake
#
# Copyright (C) 2006-2017  Alan W. Irwin
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

# Module for determining Python binding configuration options

# Options to enable Python binding
if(DEFAULT_NO_BINDINGS)
  option(ENABLE_python "Enable Python binding" OFF)
else(DEFAULT_NO_BINDINGS)
  option(ENABLE_python "Enable Python binding" ON)
endif(DEFAULT_NO_BINDINGS)

find_program(SED_EXECUTABLE sed)

if(ENABLE_python AND NOT SED_EXECUTABLE)
  message(STATUS "WARNING: "
    "Python binding fixup requires sed which was not found. Disabling Python binding")
  set(ENABLE_python OFF CACHE BOOL "Enable Python binding" FORCE)
endif(ENABLE_python AND NOT SED_EXECUTABLE)

if(ENABLE_python AND NOT BUILD_SHARED_LIBS)
  message(STATUS "WARNING: "
    "Python requires shared libraries. Disabling Python binding")
  set(ENABLE_python OFF CACHE BOOL "Enable Python binding" FORCE)
endif(ENABLE_python AND NOT BUILD_SHARED_LIBS)

if(ENABLE_python AND NOT SWIG_FOUND)
  message(STATUS "WARNING: "
    "swig not found. Disabling Python binding")
  set(ENABLE_python OFF CACHE BOOL "Enable Python binding" FORCE)
endif(ENABLE_python AND NOT SWIG_FOUND)

if(ENABLE_python)
  # Check for the Python interpreter (which also defines PYTHON_EXECUTABLE)

  # Document the user-settable value PLPLOT_PYTHON_EXACT_VERSION in the cache and set the type
  # of this cached variable.
  set(PLPLOT_PYTHON_EXACT_VERSION "" CACHE STRING "Search for this exact version of Python first")
  if(NOT PYTHONINTERP_FOUND AND
      PLPLOT_PYTHON_EXACT_VERSION AND
      NOT PLPLOT_PYTHON_EXACT_VERSION VERSION_LESS "2.0.0" AND
      PLPLOT_PYTHON_EXACT_VERSION VERSION_LESS "4.0.0"
      )
    # Check first for and exact version (if specified by the user).
    find_package(PythonInterp ${PLPLOT_PYTHON_EXACT_VERSION} EXACT)

  endif(NOT PYTHONINTERP_FOUND AND
    PLPLOT_PYTHON_EXACT_VERSION AND
    NOT PLPLOT_PYTHON_EXACT_VERSION VERSION_LESS "2.0.0" AND
    PLPLOT_PYTHON_EXACT_VERSION VERSION_LESS "4.0.0"
    )

  if(NOT PYTHONINTERP_FOUND)
    # We prefer Python 3 if it is available because that (according
    # to one Python developer, and his opinion makes sense) is
    # better maintained than Python 2.
    option(FORCE_PYTHON2 "Force Python2 even when Python 3 is present" OFF)
    if(NOT FORCE_PYTHON2)
      find_package(PythonInterp 3)
    endif(NOT FORCE_PYTHON2)
  endif(NOT PYTHONINTERP_FOUND)

  if(NOT PYTHONINTERP_FOUND)
    # Fall back to Python 2 if Python 3 not found or FORCE_PYTHON2 option is set to ON.
    find_package(PythonInterp 2)
    if(NOT PYTHONINTERP_FOUND)
      message(STATUS "WARNING: "
	"Python interpreter not found. Disabling Python binding")
      set(ENABLE_python OFF CACHE BOOL "Enable Python binding" FORCE)
    endif(NOT PYTHONINTERP_FOUND)
  endif(NOT PYTHONINTERP_FOUND)
endif(ENABLE_python)

if(ENABLE_python)
  # Check for Python libraries which defines
  #  PYTHON_LIBRARIES     = path to the Python library
  #  PYTHON_INCLUDE_PATH  = path to where Python.h is found
  find_package(PythonLibs)
  if(NOT PYTHON_LIBRARIES OR NOT PYTHON_INCLUDE_PATH)
    message(STATUS "WARNING: "
      "Python library and/or header not found. Disabling Python binding")
    set(ENABLE_python OFF CACHE BOOL "Enable Python binding" FORCE)
  endif(NOT PYTHON_LIBRARIES OR NOT PYTHON_INCLUDE_PATH)
endif(ENABLE_python)

if(ENABLE_python)
  # NUMPY_INCLUDE_PATH = path to arrayobject.h for numpy.
  #message(STATUS "DEBUG: NUMPY_INCLUDE_PATH = ${NUMPY_INCLUDE_PATH}")
  if(NOT NUMPY_INCLUDE_PATH)
    # Check for numpy installation.
    execute_process(
      COMMAND
      ${PYTHON_EXECUTABLE} -c "import numpy; print(numpy.get_include())"
      OUTPUT_VARIABLE NUMPY_INCLUDE_PATH_PARENT
      RESULT_VARIABLE NUMPY_ERR
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    if(NUMPY_ERR)
      set(NUMPY_INCLUDE_PATH)
    else(NUMPY_ERR)
      # We use the full path name (including numpy on the end), but
      # Double-check that all is well with that choice.
      find_path(
	NUMPY_INCLUDE_PATH
	arrayobject.h
	${NUMPY_INCLUDE_PATH_PARENT}/numpy
	)
    endif(NUMPY_ERR)

  endif(NOT NUMPY_INCLUDE_PATH)

  if(NOT NUMPY_INCLUDE_PATH)
    message(STATUS "WARNING: "
	"NumPy header not found. Disabling Python binding")
    set(ENABLE_python OFF CACHE BOOL "Enable Python binding" FORCE)
  endif(NOT NUMPY_INCLUDE_PATH)
endif(ENABLE_python)

if(ENABLE_python)
  # This numpy installation bug found by Geoff.
  option(EXCLUDE_PYTHON_LIBRARIES "Linux temporary workaround for numpy installation bug for non-system Python install prefix" OFF)
  if(EXCLUDE_PYTHON_LIBRARIES)
    set(PYTHON_LIBRARIES)
  endif(EXCLUDE_PYTHON_LIBRARIES)
endif(ENABLE_python)

if(ENABLE_python)
  # if CMAKE_INSTALL_EXEC_PREFIX is an empty string, must replace
  # it with "/" to make PYTHON_INSTALL_TEMPLATE an absolute path to be
  # consistent with all other installation paths.
  if(CMAKE_INSTALL_EXEC_PREFIX)
    set(PYTHON_INSTALL_TEMPLATE "${CMAKE_INSTALL_EXEC_PREFIX}")
  else(CMAKE_INSTALL_EXEC_PREFIX)
    set(PYTHON_INSTALL_TEMPLATE "/")
  endif(CMAKE_INSTALL_EXEC_PREFIX)

  # N.B. This is a nice way to obtain all sorts of Python information
  # using distutils.
  execute_process(
    COMMAND
    ${PYTHON_EXECUTABLE} -c "from distutils import sysconfig; print(sysconfig.get_python_lib(1,0,prefix='${PYTHON_INSTALL_TEMPLATE}'))"
    OUTPUT_VARIABLE _python_instdir
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )

  # Convert result to a cached variable.
  set(
    PYTHON_INSTDIR
    "${_python_instdir}"
    CACHE PATH "PLplot install location for Python extension modules"
    )
  list(APPEND INSTALL_LOCATION_VARIABLES_LIST PYTHON_INSTDIR)

  # Get the Python version.
  execute_process(
    COMMAND
    ${PYTHON_EXECUTABLE} -c "import sys; print('%s.%s.%s' % sys.version_info[0:3])"
    OUTPUT_VARIABLE PYTHON_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
    )
  message(STATUS "PYTHON_VERSION = ${PYTHON_VERSION}")

  if(("1.3.38" VERSION_LESS ${SWIG_VERSION}) AND ("2.6.0" VERSION_LESS ${PYTHON_VERSION}))
      message(STATUS "Building Python binding with plsmem() support")
      set(PYTHON_HAVE_PYBUFFER ON)
  else(("1.3.38" VERSION_LESS ${SWIG_VERSION}) AND ("2.6.0" VERSION_LESS ${PYTHON_VERSION}))
      message(STATUS "WARNING: Python and Swig versions do not support building Python binding with plsmem() support")
      set(PYTHON_HAVE_PYBUFFER OFF)
  endif(("1.3.38" VERSION_LESS ${SWIG_VERSION}) AND ("2.6.0" VERSION_LESS ${PYTHON_VERSION}))

endif(ENABLE_python)
