find_package(PkgConfig)
pkg_check_modules(PC_ZLIB  zlib)

find_path(UNIBREAK_INCLUDE_DIRS
    NAMES wordbreak.h
    HINTS ${PC_ZLIB_INCLUDEDIR}
    ${PC_ZLIB_INCLUDE_DIRS}
)

find_library(UNIBREAK_LIBRARIES
    NAMES unibreak
    HINTS ${PC_ZLIB_LIBDIR}
    ${PC_ZLIB_LIBRARY_DIRS}
)

#message("UNIBREAK_LIBRARIES=${UNIBREAK_LIBRARIES} UNIBREAK_INCLUDE_DIRS=${UNIBREAK_INCLUDE_DIRS} PC_UNIBREAK_VERSION=${PC_UNIBREAK_VERSION}")
if(UNIBREAK_INCLUDE_DIRS AND UNIBREAK_LIBRARIES)
    set(UNIBREAK_FOUND TRUE)
    set(UNIBREAK_LIBRARY ${UNIBREAK_LIBRARIES})
    set(UNIBREAK_INCLUDE_DIR ${UNIBREAK_INCLUDE_DIRS})
    set(UNIBREAK_VERSION "5.0.0")
endif()

include(FindPackageHandleStandardArgs)
#FIND_PACKAGE_HANDLE_STANDARD_ARGS(zlib REQUIRED_VARS UNIBREAK_INCLUDE_DIRS UNIBREAK_LIBRARIES 
#	FOUND_VAR UNIBREAK_FOUND VERSION_VAR UNIBREAK_VERSION)

mark_as_advanced(UNIBREAK_LIBRARIES UNIBREAK_LIBRARY UNIBREAK_INCLUDE_DIRS UNIBREAK_INCLUDE_DIR UNIBREAK_FOUND UNIBREAK_VERSION)
