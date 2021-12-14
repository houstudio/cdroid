# cmake/modules/pdf.cmake
#
# Copyright (C) 2006 Werner Smekal
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

# Module for determining all configuration variables related to the pdf device
# driver.
# The following variables are set/modified:
# PLD_pdf		  - ON means the pdf device is enabled.
# pdf_COMPILE_FLAGS	  - individual COMPILE_FLAGS required to compile pdf
# 			    device.
# pdf_LINK_FLAGS	  - individual LINK_FLAGS for dynamic pdf device.
# pdf_RPATH               - RPATH directory list for pdf device driver.
# DRIVERS_LINK_FLAGS	  - list of LINK_FLAGS for all static devices.

if(PLD_pdf)
  message(STATUS "Looking for haru pdf header and library")
  find_package(hpdf)
  if(hpdf_FOUND)
    message(STATUS "Looking for haru pdf header and library - found")
    if(WIN32_OR_CYGWIN AND NOT CMAKE_COMPILER_IS_GNUCC)
      set(pdf_COMPILE_FLAGS "-I${hpdf_INCLUDE_DIRS} -DHPDF_DLL")
    elseif(WIN32_OR_CYGWIN AND CMAKE_COMPILER_IS_GNUCC)
      set(pdf_COMPILE_FLAGS "-I${hpdf_INCLUDE_DIRS} -DHPDF_DLL_CDECL")
    else(WIN32_OR_CYGWIN AND NOT CMAKE_COMPILER_IS_GNUCC)
      set(pdf_COMPILE_FLAGS "-I${hpdf_INCLUDE_DIRS}")
    endif(WIN32_OR_CYGWIN AND NOT CMAKE_COMPILER_IS_GNUCC)
    set(pdf_LINK_FLAGS "${hpdf_LIBRARIES}")
    set(pdf_RPATH ${hpdf_LIBRARY_DIRS})
    filter_rpath(pdf_RPATH)
    set(DRIVERS_LINK_FLAGS ${DRIVERS_LINK_FLAGS} ${pdf_LINK_FLAGS})
    # libharu version must be >= 2.3.0.
    # Determine that version in cross-platform way.
    set(check_libharu_version_source "
#include \"hpdf.h\"
int main(void)
{
// True if version is 2.3.0 or later....
#if defined(HPDF_MAJOR_VERSION) && defined(HPDF_MINOR_VERSION) && defined(HPDF_BUGFIX_VERSION) \
&&  (HPDF_BUGFIX_VERSION + 1000*(HPDF_MINOR_VERSION + 1000*HPDF_MAJOR_VERSION)) >= 2003000
// Return success
  return 0;
#else
// Return failure
  return 1;
#endif
}
")
    include(CheckCSourceRuns)

    cmake_push_check_state()
    list(APPEND CMAKE_REQUIRED_INCLUDES ${hpdf_INCLUDE_DIRS})
    message(STATUS "Checking whether libharu version >= 2.3.0")
    check_c_source_runs("${check_libharu_version_source}" LIBHARU_VERSION_LARGE_ENOUGH)
    cmake_pop_check_state()
    if(NOT LIBHARU_VERSION_LARGE_ENOUGH)
      message(STATUS
	"WARNING: libharu version is less than 2.3.0 (required for handling medium-large fonts and other fixes) so setting PLD_pdf to OFF."
	)
      set(PLD_pdf OFF CACHE BOOL "Enable pdf device" FORCE)
    endif(NOT LIBHARU_VERSION_LARGE_ENOUGH)
  else(hpdf_FOUND)
    message(STATUS "Looking for haru pdf header and library - not found")
    message(STATUS "WARNING: Setting PLD_pdf to OFF.")
    set(PLD_pdf OFF CACHE BOOL "Enable pdf device" FORCE)
  endif(hpdf_FOUND)
endif(PLD_pdf)
