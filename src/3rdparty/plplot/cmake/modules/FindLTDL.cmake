# - Find ltdl header and library
#

# This module defines
#  LTDL_FOUND, if false, do not try to use LTDL.
#  LTDL_INCLUDE_DIR, where to find ltdl.h.
#  LTDL_LIBRARIES, the libraries to link against to use libltdl.
#  LTDL_LIBRARY_DIR, the directory where libltdl is found.

find_path(LTDL_INCLUDE_DIR ltdl.h /usr/local/include /usr/include)

if(LTDL_INCLUDE_DIR)
  find_library(LTDL_LIBRARIES
  NAMES ltdl
  PATHS /usr/local/lib /usr/lib
  )
  if(LTDL_LIBRARIES)

    set(LTDL_LIBRARY_DIR "")
    get_filename_component(LTDL_LIBRARY_DIR ${LTDL_LIBRARIES} PATH)

    # Define useful internal macro for finding and checking a library
    include(CheckLibraryExists)
    macro(_find_check_libs _library_name _function_name _lib _have_lib)
      find_library(
      ${_lib}
      NAMES ${_library_name}
      PATHS /usr/local/lib /usr/lib
      )
#      message("_library_name = ${_library_name}")
#      message("${_lib} = ${${_lib}}")
      if(${_lib})
        get_filename_component(_path ${${_lib}} PATH)
        check_library_exists(
	${_library_name}
	${_function_name}
	${_path}
	${_have_lib}
	)
      endif(${_lib})
    endmacro(_find_check_libs)

    if(NOT NON_TRANSITIVE)
      # Find and check platform-dependent libraries which
      # libltdl depends upon using model given
      # for determining LIBADD_DL in $prefix/share/libtool/libltdl/acinclude.m4

      # Look for three exclusive alternatives:
      # (dld shl_load; dl dlopen; svdl dlopen)
      # N.B. all the different ADDLIB variables have to be different because
      # of caching of results in the macro.
      _find_check_libs(dld shl_load LTDL_LTDL_ADDLIB0 LTDL_LTDL_HAVE_ADDLIB0)
      if(LTDL_LTDL_HAVE_ADDLIB0)
        set(LTDL_LIBRARIES ${LTDL_LIBRARIES} ${LTDL_LTDL_ADDLIB0})
        set(LTDL_FOUND ON)
      else(LTDL_LTDL_HAVE_ADDLIB0)
        _find_check_libs(dl dlopen LTDL_LTDL_ADDLIB1 LTDL_LTDL_HAVE_ADDLIB1)
        if(LTDL_LTDL_HAVE_ADDLIB1)
          set(LTDL_LIBRARIES ${LTDL_LIBRARIES} ${LTDL_LTDL_ADDLIB1})
          set(LTDL_FOUND ON)
        else(LTDL_LTDL_HAVE_ADDLIB1)
          _find_check_libs(svdl dlopen LTDL_LTDL_ADDLIB2 LTDL_LTDL_HAVE_ADDLIB2)
          if(LTDL_LTDL_HAVE_ADDLIB2)
            set(LTDL_LIBRARIES ${LTDL_LIBRARIES} ${LTDL_LTDL_ADDLIB2})
            set(LTDL_FOUND ON)
          else(LTDL_LTDL_HAVE_ADDLIB2)
	    set(LTDL_FOUND OFF)
          endif(LTDL_LTDL_HAVE_ADDLIB2)
        endif(LTDL_LTDL_HAVE_ADDLIB1)
      endif(LTDL_LTDL_HAVE_ADDLIB0)

      # (dld dld_link) appended to any that have been found before.
      _find_check_libs(dld dld_link LTDL_LTDL_ADDLIB3 LTDL_LTDL_HAVE_ADDLIB3)
      if(LTDL_LTDL_HAVE_ADDLIB3)
        set(LTDL_LIBRARIES ${LTDL_LIBRARIES} ${LTDL_LTDL_ADDLIB3})
        set(LTDL_FOUND ON)
      endif(LTDL_LTDL_HAVE_ADDLIB3)
    else(NOT NON_TRANSITIVE)
      set(LTDL_FOUND ON)
    endif(NOT NON_TRANSITIVE)

  endif(LTDL_LIBRARIES)
endif(LTDL_INCLUDE_DIR)

if(LTDL_FOUND)
  if(NOT LTDL_FIND_QUIETLY)
    message(STATUS "Found LTDL: ${LTDL_LIBRARIES}")
  endif(NOT LTDL_FIND_QUIETLY)
else(LTDL_FOUND)
  if(LTDL_FIND_REQUIRED)
    message(FATAL_ERROR "Could not find LTDL library")
  endif(LTDL_FIND_REQUIRED)
endif(LTDL_FOUND)

mark_as_advanced(
LTDL_LTDL_ADDLIB0 LTDL_LTDL_HAVE_ADDLIB0
LTDL_LTDL_ADDLIB1 LTDL_LTDL_HAVE_ADDLIB1
LTDL_LTDL_ADDLIB2 LTDL_LTDL_HAVE_ADDLIB2
LTDL_LTDL_ADDLIB3 LTDL_LTDL_HAVE_ADDLIB3
)
