set(EXPAT_INCLUDE_DIR  ${CMAKE_BINARY_DIR}/include)
set(EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIR})
set(EXPAT_LIBRARY expat)
set(EXPAT_LIBRARIES ${EXPAT_LIBRARY})
set(EXPAT_INCLUDE_DIRS ${EXPAT_INCLUDE_DIR})
set(EXPAT_FOUND TRUE)

find_package(PkgConfig)
pkg_check_modules(PC_CDROID QUIET cdroid)

find_path(CDROID_INCLUDE_DIRS
    NAMES view/view.h
    HINTS ${PC_CDROID_INCLUDEDIR}
          ${PC_CDROID_INCLUDE_DIRS}
    PATH_SUFFIXES gui
)

find_library(CDROID_LIBRARIES
    NAMES cdroid
    HINTS ${PC_CDROID_LIBDIR}
          ${PC_CDROID_LIBRARY_DIRS}
)

if (CDROID_INCLUDE_DIRS)
    if (EXISTS "${CDROID_INCLUDE_DIRS}/cdroid-version.h")
        file(READ "${CDROID_INCLUDE_DIRS}/cdroid-version.h" CDROID_VERSION_CONTENT)

        string(REGEX MATCH "#define +CDROID_VERSION_MAJOR +([0-9]+)" _dummy "${CDROID_VERSION_CONTENT}")
        set(CDROID_VERSION_MAJOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define +CDROID_VERSION_MINOR +([0-9]+)" _dummy "${CDROID_VERSION_CONTENT}")
        set(CDROID_VERSION_MINOR "${CMAKE_MATCH_1}")

        string(REGEX MATCH "#define +CDROID_VERSION_MICRO +([0-9]+)" _dummy "${CDROID_VERSION_CONTENT}")
        set(CDROID_VERSION_MICRO "${CMAKE_MATCH_1}")

        set(CDROID_VERSION "${CDROID_VERSION_MAJOR}.${CDROID_VERSION_MINOR}.${CDROID_VERSION_MICRO}")
    endif ()
endif ()

if ("${Cdroid_FIND_VERSION}" VERSION_GREATER "${CDROID_VERSION}")
    message(FATAL_ERROR "Required version (" ${Cdroid_FIND_VERSION} ") is higher than found version (" ${CDROID_VERSION} ")")
endif ()

include(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(CDROID REQUIRED_VARS CDROID_INCLUDE_DIRS CDROID_LIBRARIES
                                        VERSION_VAR CDROID_VERSION)

mark_as_advanced(
    CDROID_INCLUDE_DIRS
    CDROID_LIBRARIES
)

