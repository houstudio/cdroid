# Find haru pdf header and library.
#

# This module defines the following uncached variables:
#  hpdf_FOUND, if false, do not try to use haru pdf.
#  hpdf_INCLUDE_DIRS, where to find hpdf.h.
#  hpdf_LIBRARIES, the libraries to link against to use haru pdf
#  hpdf_LIBRARY_DIRS, the directory where libhpdf (either shared or static)
#  is found.

# Give preference to header found in hpdf subdirectory (which is the
# result of a build and install of upstream libharu), then if not
# found look for the case of no such subdirectory (which is how the
# Debian system version of libharu headers is installed).
find_path(hpdf_INCLUDE_DIR hpdf.h PATHS /usr/local/include /usr/include PATH_SUFFIXES hpdf)
find_path(hpdf_INCLUDE_DIR hpdf.h PATHS /usr/local/include /usr/include)

if(hpdf_INCLUDE_DIR)
  find_library(hpdf_LIBRARY
  NAMES hpdf
  PATHS /usr/local/lib /usr/lib
  )
  if(hpdf_LIBRARY)
    set(hpdf_LIBRARY_DIR "")
    get_filename_component(hpdf_LIBRARY_DIRS ${hpdf_LIBRARY} PATH)
    # Set uncached variables as per standard.
    set(hpdf_FOUND ON)
    set(hpdf_INCLUDE_DIRS ${hpdf_INCLUDE_DIR})
    set(hpdf_LIBRARIES ${hpdf_LIBRARY})
  endif(hpdf_LIBRARY)
endif(hpdf_INCLUDE_DIR)

if(hpdf_FOUND)
  if(NOT hpdf_FIND_QUIETLY)
    message(STATUS "Findhpdf: Found haru header directory, ${hpdf_INCLUDE_DIR}, and library, ${hpdf_LIBRARIES}.")
  endif(NOT hpdf_FIND_QUIETLY)
else(hpdf_FOUND)
  if(hpdf_FIND_REQUIRED)
    message(FATAL_ERROR "Findhpdf: Could not find haru (hpdf) header and/or library.")
  endif(hpdf_FIND_REQUIRED)
endif(hpdf_FOUND)
