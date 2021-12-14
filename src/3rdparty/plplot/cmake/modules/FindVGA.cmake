# Find vga-related library information for Linux

# This module defines the following uncached variables:
#  VGA_FOUND, if false, do not try to use VGA.
#  VGA_INCLUDE_DIRS, where to find vga.h.
#  VGA_LIBRARIES, the libraries to link against to use the vga library
#  VGA_LIBRARY_DIRS, the directory where the vga library is found.

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
find_path(VGA_INCLUDE_DIR vga.h /usr/local/include /usr/include)

if(VGA_INCLUDE_DIR)
  find_library(VGA_LIBRARY
  NAMES vga
  PATHS /usr/local/lib /usr/lib
  )
  if(VGA_LIBRARY)
    set(VGA_LIBRARY_DIR "")
    get_filename_component(VGA_LIBRARY_DIRS ${VGA_LIBRARY} PATH)
    # Set uncached variables as per standard.
    set(VGA_FOUND ON)
    set(VGA_INCLUDE_DIRS ${VGA_INCLUDE_DIR})
    set(VGA_LIBRARIES ${VGA_LIBRARY})
  endif(VGA_LIBRARY)
endif(VGA_INCLUDE_DIR)

if(VGA_FOUND)
  if(NOT VGA_FIND_QUIETLY)
    message(STATUS "FindVGA: Found both vga.h and the vga library")
  endif(NOT VGA_FIND_QUIETLY)
else(VGA_FOUND)
  if(VGA_FIND_REQUIRED)
    message(FATAL_ERROR "FindVGA: Could not find vga.h and/or the vga library")
  endif(VGA_FIND_REQUIRED)
endif(VGA_FOUND)
endif(CMAKE_SYSTEM_NAME STREQUAL "Linux")
