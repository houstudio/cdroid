
find_package(PkgConfig)
pkg_check_modules(PC_WEBP libwebpmux)

find_path(WEBP_INCLUDE_DIR
    NAMES demux.h
    HINTS ${PC_WEBP_INCLUDE_DIRS}
    ${PC_WEBP_INCLUDEDIR}
)

find_library(WEBP_LIBRARY
    NAMES webp
    HINTS ${PC_WEBP_LIBDIR}
    ${PC_WEBP_LIBRARY_DIRS}
)
find_library(WEBP_DEMUX_LIBRARY
    NAMES webpdemux
    HINTS ${PC_WEBP_LIBDIR}
    ${PC_WEBP_LIBRARY_DIRS}
)
find_library(WEBP_DECODER_LIBRARY
    NAMES webpdecoder
    HINTS ${PC_WEBP_LIBDIR}
    ${PC_WEBP_LIBRARY_DIRS}
)
find_library(WEBP_SHARPYUV_LIBRARY
    NAMES sharpyuv
    HINTS ${PC_WEBP_LIBDIR}
    ${PC_WEBP_LIBRARY_DIRS}
)
find_library(WEBP_MUX_LIBRARY
    NAMES webpmux
    HINTS ${PC_WEBP_LIBDIR}
    ${PC_WEBP_LIBRARY_DIRS}
)

if(WEBP_INCLUDE_DIR AND WEBP_LIBRARY)
    set(WEBP_FOUND TRUE)
    set(WEBP_INCLUDE_DIRS ${WEBP_INCLUDE_DIR})
    set(WEBP_LIBRARIES ${WEBP_LIBRARY} ${WEBP_DEMUX_LIBRARY} ${WEBP_DECODER_LIBRARY} 
	    ${WEBP_SHARPYUV_LIBRARY} ${WEBP_MUX_LIBRARY})
    set(WEBP_VERSION ${PC_WEBP_VERSION})
    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(WEBP  FOUND_VAR WEBP_FOUND VERSION_VAR WEBP_VERSION 
	    REQUIRED_VARS WEBP_LIBRARIES WEBP_LIBRARY WEBP_INCLUDE_DIRS WEBP_INCLUDE_DIR)
    mark_as_advanced(WEBP_LIBRARIES WEBP_LIBRARY WEBP_INCLUDE_DIRS WEBP_INCLUDE_DIR)
    if(NOT TARGET WebP::webp)
	add_library(WebP::webp UNKNOWN IMPORTED)
        set_target_properties(WebP::webp PROPERTIES
		INTERFACE_INCLUDE_DIRECTORIES "${WEBP_INCLUDE_DIRS}")
	set_property(TARGET WebP::webp APPEND PROPERTY  IMPORTED_LOCATION ${WEBP_LIBRARY})
    endif()
    if(NOT TARGET WebP::webpdecoder)
	add_library(WebP::webpdecoder UNKNOWN IMPORTED)
	set_property(TARGET WebP::webpdecoder PROPERTY  IMPORTED_LOCATION ${WEBP_DECODER_LIBRARY})
    endif()
    if(NOT TARGET WebP::webpdemux)
	add_library(WebP::webpdemux UNKNOWN IMPORTED)
	set_property(TARGET WebP::webpdemux PROPERTY IMPORTED_LOCATION ${WEBP_DEMUX_LIBRARY})
    endif()
    if(NOT TARGET WebP::libwebpmux)
	add_library(WebP::libwebpmux UNKNOWN IMPORTED)
	set_property(TARGET WebP::libwebpmux PROPERTY IMPORTED_LOCATION ${WEBP_MUX_LIBRARY})
    endif()
    if(NOT TARGET WebP::sharpyuv)
	add_library(WebP::sharpyuv UNKNOWN IMPORTED)
	set_property(TARGET WebP::sharpyuv PROPERTY IMPORTED_LOCATION ${WEBP_SHARPYUV_LIBRARY})
    endif()
endif()

