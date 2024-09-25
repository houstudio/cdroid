# Copyright (c) 2012, Intel Corporation
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# * Redistributions of source code must retain the above copyright notice, this
#   list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright notice,
#   this list of conditions and the following disclaimer in the documentation
#   and/or other materials provided with the distribution.
# * Neither the name of Intel Corporation nor the names of its contributors may
#   be used to endorse or promote products derived from this software without
#   specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Try to find Harfbuzz include and library directories.
#
# After successful discovery, this will set for inclusion where needed:
# HARFBUZZ_INCLUDE_DIRS - containg the HarfBuzz headers
# HARFBUZZ_LIBRARIES - containg the HarfBuzz library

include(FindPkgConfig)

pkg_check_modules(PC_HARFBUZZ harfbuzz>=0.9.7)

find_path(HARFBUZZ_INCLUDE_DIRS NAMES hb.h
    HINTS ${PC_HARFBUZZ_INCLUDE_DIRS} ${PC_HARFBUZZ_INCLUDEDIR}
)

find_library(HARFBUZZ_LIBRARIES NAMES harfbuzz
    HINTS ${PC_HARFBUZZ_LIBRARY_DIRS} ${PC_HARFBUZZ_LIBDIR}
)

if (HARFBUZZ_INCLUDE_DIRS)
    if (EXISTS "${HARFBUZZ_INCLUDE_DIRS}/hb-version.h")
        file(READ "${HARFBUZZ_INCLUDE_DIRS}/hb-version.h" HB_VERSION_CONTENT)

	string(REGEX MATCH "#define +HB_VERSION_MAJOR +([0-9]+)" _dummy "${HB_VERSION_CONTENT}")
	set(HB_VERSION_MAJOR "${CMAKE_MATCH_1}")

	string(REGEX MATCH "#define +HB_VERSION_MINOR +([0-9]+)" _dummy "${HB_VERSION_CONTENT}")
	set(HB_VERSION_MINOR "${CMAKE_MATCH_1}")

	string(REGEX MATCH "#define +HB_VERSION_MICRO +([0-9]+)" _dummy "${HB_VERSION_CONTENT}")
	set(HB_VERSION_MICRO "${CMAKE_MATCH_1}")

	set(HARFBUZZ_VERSION "${HB_VERSION_MAJOR}.${HB_VERSION_MINOR}.${HB_VERSION_MICRO}")
    endif ()
endif ()

message("HARFBUZZ_LIBRARIES=${HARFBUZZ_LIBRARIES} HARFBUZZ_INCLUDE_DIRS=${HARFBUZZ_INCLUDE_DIRS} HARFBUZZ_VERSION=${HARFBUZZ_VERSION}")
# HarfBuzz 0.9.18 split ICU support into a separate harfbuzz-icu library.
if ("${PC_HARFBUZZ_VERSION}" VERSION_GREATER "0.9.17")
    pkg_check_modules(PC_HARFBUZZ_ICU harfbuzz-icu>=0.9.18 REQUIRED)
    find_library(HARFBUZZ_ICU_LIBRARIES NAMES harfbuzz-icu
        HINTS ${PC_HARFBUZZ_ICU_LIBRARY_DIRS} ${PC_HARFBUZZ_ICU_LIBDIR}
    )
    list(APPEND HARFBUZZ_LIBRARIES "${HARFBUZZ_ICU_LIBRARIES}")
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(HarfBuzz DEFAULT_MSG HARFBUZZ_INCLUDE_DIRS HARFBUZZ_LIBRARIES HARFBUZZ_FOUND)

mark_as_advanced(
    HARFBUZZ_INCLUDE_DIRS
    HARFBUZZ_LIBRARIES
)
if( HARFBUZZ_INCLUDE_DIRS AND NOT HARFBUZZ_LIBRARIES)
   set(HARFBUZZ_LIBRARIES harfbuzz)
   set(HARFBUZZ_FOUND TRUE)
   if(NOT TARGET harfbuzz::harfbuzz)
      add_library(harfbuzz::harfbuzz UNKNOWN IMPORTED)
      set_target_properties(harfbuzz::harfbuzz PROPERTIES
	      INTERFACE_INCLUDE_DIRECTORIES "${HARFBUZZ_INCLUDE_DIRS}")

      if(HARFBUZZ_LIBRARY_RELEASE)
        set_property(TARGET harfbuzz::harfbuzz APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(harfbuzz::harfbuzz PROPERTIES
		IMPORTED_LOCATION_RELEASE "${HARFBUZZ_LIBRARY_RELEASE}")
      endif()

      if(HARFBUZZ_LIBRARY_DEBUG)
        set_property(TARGET harfbuzz::harfbuzz APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(harfbuzz::harfbuzz PROPERTIES
		IMPORTED_LOCATION_DEBUG "${HARFBUZZ_LIBRARY_DEBUG}")
      endif()
   endif()
endif()
set(HarfBuzz_INCLUDE_DIRS ${HARFBUZZ_INCLUDE_DIRS})
set(HarfBuzz_LIBRARIES ${HARFBUZZ_LIBRARIES})
set(HarfBuzz_LIBRARY ${HARFBUZZ_LIBRARIES})
#message(FATAL_ERROR "***HARFBUZZ_INCLUDE_DIRS=${HARFBUZZ_INCLUDE_DIRS}    HARFBUZZ_LIBRARIES=${HARFBUZZ_LIBRARIES}")
