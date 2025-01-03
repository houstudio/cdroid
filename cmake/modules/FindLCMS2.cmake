find_package(PkgConfig)
pkg_check_modules(PC_LCMS2 lcms2)

find_path(LCMS2_INCLUDE_DIRS
    NAMES lcms2.h
    HINTS ${PC_LCMS2_INCLUDEDIR}
    ${PC_LCMS2_INCLUDE_DIRS}
)

find_library(LCMS2_LIBRARIES
    NAMES lcms2
    HINTS ${PC_LCMS2_LIBDIR}
    ${PC_LCMS2_LIBRARY_DIRS}
)

if(LCMS2_INCLUDE_DIRS AND LCMS2_LIBRARIES)
    set(LCMS2_FOUND TRUE)
    set(LCMS2_LIBRARY ${LCMS2_LIBRARIES})
    set(LCMS2_INCLUDE_DIR ${LCMS2_INCLUDE_DIRS})
    set(LCMS2_VERSION ${PC_LCMS2_VERSION})
    include(FindPackageHandleStandardArgs)
    FIND_PACKAGE_HANDLE_STANDARD_ARGS(LCMS2 FOUND_VAR LCMS2_FOUND VERSION_VAR LCMS2_VERSION
	    REQUIRED_VARS LCMS2_INCLUDE_DIRS LCMS2_INCLUDE_DIR LCMS2_LIBRARIES LCMS2_LIBRARY)

    mark_as_advanced(LCMS2_LIBRARIES LCMS2_LIBRARY LCMS2_INCLUDE_DIRS LCMS2_INCLUDE_DIR LCMS2_FOUND LCMS2_VERSION)

    if(NOT TARGET lcms2::lcms2)
        add_library(lcms2::lcms2 UNKNOWN IMPORTED)
        set_target_properties(lcms2::lcms2 PROPERTIES
          INTERFACE_COMPILE_DEFINITIONS "${_LCMS_COMPILE_DEFINITIONS}"
          INTERFACE_INCLUDE_DIRECTORIES "${LCMS_INCLUDE_DIRS}"
          )

      if(EXISTS "${LCMS2_LIBRARY}")
          set_target_properties(lcms2::lcms2 PROPERTIES
            IMPORTED_LINK_INTERFACE_LANGUAGES "C"
            IMPORTED_LOCATION "${LCMS2_LIBRARY}")
        endif()
    endif()
endif()

