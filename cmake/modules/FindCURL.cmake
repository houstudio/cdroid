# Finds the International Components for Unicode (CURL) Library
#
#  CURL_FOUND          - True if CURL found.
#  CURL_INCLUDE_DIRS   - Directory to include to get CURL headers
#                       Note: always include CURL headers as, e.g.,
#                       unicode/utypes.h
#  CURL_LIBRARIES      - Libraries to link against for the common CURL

find_package(PkgConfig)
pkg_check_modules(PC_CURL libcurl)

# Look for the header file.
find_path(
    CURL_INCLUDE_DIR
    NAMES curl/curl.h
    HINTS ${PC_CURL_INCLUDE_DIRS}
    ${PC_CURL_INCLUDEDIR}
    DOC "Include directory for the CURL library")

find_library(CURL_LIBRARY
    NAMES curl curl-d curld
    HINTS ${PC_CURL_LIBDIR}
    ${PC_CURL_LIBRARY_DIRS}
)

# Copy the results to the output variables.
if (CURL_INCLUDE_DIR AND CURL_LIBRARY)
    set(CURL_FOUND 1)
    set(CURL_LIBRARIES ${CURL_LIBRARY})
    set(CURL_INCLUDE_DIRS ${CURL_INCLUDE_DIR})
    set(CURL_VERSION ${PC_CURL_VERSION})
    if(NOT TARGET CURL::libcurl)
      add_library(CURL::libcurl UNKNOWN IMPORTED)
      set_target_properties(CURL::libcurl PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${CURL_INCLUDE_DIRS}")

      if(EXISTS "${CURL_LIBRARY}")
        set_target_properties(CURL::libcurl PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES "C"
          IMPORTED_LOCATION "${CURL_LIBRARY}")
      endif()
      if(CURL_LIBRARY_RELEASE)
        set_property(TARGET CURL::libcurl APPEND PROPERTY
          IMPORTED_CONFIGURATIONS RELEASE)
        set_target_properties(CURL::libcurl PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES "C"
          IMPORTED_LOCATION_RELEASE "${CURL_LIBRARY_RELEASE}")
      endif()
      if(CURL_LIBRARY_DEBUG)
        set_property(TARGET CURL::libcurl APPEND PROPERTY
          IMPORTED_CONFIGURATIONS DEBUG)
        set_target_properties(CURL::libcurl PROPERTIES
          IMPORTED_LINK_INTERFACE_LANGUAGES "C"
          IMPORTED_LOCATION_DEBUG "${CURL_LIBRARY_DEBUG}")
      endif()
  endif()
else ()
    set(CURL_FOUND 0)
    set(CURL_INCLUDE_DIRS)
    set(CURL_VERSION)
endif ()

if (CURL_FOUND)
    mark_as_advanced(CURL_INCLUDE_DIR CURL_INCLUDE_DIRS CURL_LIBRARY CURL_LIBRARIES)
    message(STATUS "Found CURL header files in ${CURL_INCLUDE_DIRS} ver=${CURL_VERSION}")
else ()
    message("Could not find CURL")
endif ()
