# - Try to find the AGG graphics library
# Once done this will define in uncached variables
#
#  AGG_FOUND - system has AGG
#  AGG_INCLUDE_DIRS - the AGG include directories
#  AGG_DEFINITIONS - Compiler switches required for using AGG
#  AGG_LIBRARIES - Link these to use AGG
#

if(PKG_CONFIG_EXECUTABLE)
  # Use pkg-config (if available) to provide location of AGG headers and
  # libraries and also provide basis for defining AGG_DEFINITIONS.
  pkg_check_pkgconfig(libagg _AGGIncDir _AGGLinkDir _AGGLinkFlags _AGGCflags _AGGVersion _AGG)
  #message(STATUS "_AGGIncDir = ${_AGGIncDir}")
  #message(STATUS "_AGGLinkDir = ${_AGGLinkDir}")
  #message(STATUS "_AGGLinkFlags = ${_AGGLinkFlags}")
  #message(STATUS "_AGGCflags = ${_AGGCflags}")
endif(PKG_CONFIG_EXECUTABLE)

if(_AGGLinkFlags)
  find_path(AGG_INCLUDE_DIR agg2/agg_pixfmt_gray.h
    ${_AGGIncDir}
    )

  # Prefer _pic variant of library name for shared libraries case, and
  # plain name for static libraries case.
  if(BUILD_SHARED_LIBS)
    set(AGGLIB_NAMES = "agg_pic;agg")
    set(AGGFONTLIB_NAMES = "aggfontfreetype_pic;aggfontfreetype")
  else(BUILD_SHARED_LIBS)
    set(AGGLIB_NAMES = "agg;agg_pic")
    set(AGGFONTLIB_NAMES = "aggfontfreetype;aggfontfreetype_pic;")
  endif(BUILD_SHARED_LIBS)
  find_library(AGG_LIBRARY NAMES ${AGGLIB_NAMES}
    PATHS
    ${_AGGLinkDir}
    )
  find_library(AGGFONT_LIBRARY NAMES ${AGGFONTLIB_NAMES}
    PATHS
    ${_AGGLinkDir}
    /usr/local/lib
    /usr/lib
    )

  mark_as_advanced(AGG_INCLUDE_DIR AGG_LIBRARY AGGFONT_LIBRARY)

  # Set uncached variable AGG_LIBRARIES (needed by user and also
  # by FIND_PACKAGE_HANDLE_STANDARD_ARGS)
  if(AGG_LIBRARY AND AGGFONT_LIBRARY)
    set(AGG_LIBRARIES ${AGG_LIBRARY} ${AGGFONT_LIBRARY})
  elseif(AGG_LIBRARY)
    set(AGG_LIBRARIES ${AGG_LIBRARY})
  endif(AGG_LIBRARY AND AGGFONT_LIBRARY)
endif(_AGGLinkFlags)

# Standard 2.6.x method of handling QUIETLY and REQUIRED arguments and set
# AGG_FOUND to TRUE if all listed variables are TRUE
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(AGG DEFAULT_MSG AGG_LIBRARIES AGG_INCLUDE_DIR)

if(AGG_FOUND)
  # Set additional uncached variables that users of this module should use.

  set(AGG_INCLUDE_DIRS ${AGG_INCLUDE_DIR})

  # Blank-delimited is required.
  string(REGEX REPLACE ";" " " AGG_DEFINITIONS "${_AGGCflags}")

endif(AGG_FOUND)
