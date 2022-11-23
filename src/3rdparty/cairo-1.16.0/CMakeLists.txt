project(cairo)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR}/cmake)
find_package(Pixman REQUIRED)
find_package(PNG REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Freetype)
find_package(Threads)
find_package(Fontconfig)
find_package(OpenGL)

include(Configure_config.cmake)
include(Configure_features.cmake)
include_directories(${PROJECT_BINARY_DIR})

if(CAIRO_HAS_PTHREAD)
    list(APPEND CAIRO_LIBS -lpthread)
endif()

if (WIN32)
    set(CAIRO_LIBS gdi32 msimg32 user32 winmm)
endif()

include_directories(${PIXMAN_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
list(APPEND CAIRO_LIBS  ${PIXMAN_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY})
if(FONTCONFIG_FOUND)
    list(APPEND CAIRO_LIBS ${FONTCONFIG_LIBRARY})
    include_directories(${FONTCONFIG_INCLUDE_DIRS})
endif()

if(FREETYPE_FOUND)
    find_package(BZip2)
    include_directories(${FREETYPE_INCLUDE_DIRS})
    include_directories(${BZIP2_INCLUDE_DIR})
    list(APPEND CAIRO_LIBS ${FREETYPE_LIBRARIES} ${BZIP2_LIBRARIES})
endif()

if(OPENGL_FOUND)
    list(APPEND CAIRO_LIBS ${OPENGL_LIBRARIES})
    include_directories(OPENGL_INCLUDE_DIR)
endif()

include_directories(src)
add_subdirectory(src)
if (NOT CAIRO_VERSION)
	file(READ "${CMAKE_CURRENT_SOURCE_DIR}/src/cairo-version.h" CAIRO_VERSION_CONTENT)
        string(REGEX MATCH "#define +CAIRO_VERSION_MAJOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        set(CAIRO_VERSION_MAJOR "${CMAKE_MATCH_1}")
        string(REGEX MATCH "#define +CAIRO_VERSION_MINOR +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        set(CAIRO_VERSION_MINOR "${CMAKE_MATCH_1}")
        string(REGEX MATCH "#define +CAIRO_VERSION_MICRO +([0-9]+)" _dummy "${CAIRO_VERSION_CONTENT}")
        set(CAIRO_VERSION_MICRO "${CMAKE_MATCH_1}")
        set(CAIRO_VERSION "${CAIRO_VERSION_MAJOR}.${CAIRO_VERSION_MINOR}.${CAIRO_VERSION_MICRO}")
endif(NOT CAIRO_VERSION)
set(PKGCONFIG_REQUIRES "Requires")
set(CAIRO_REQUIRES ${CAIRO_LIBS})
set(CAIRO_NONPKGCONFIG_LIBS ${ZLIB_LIBRARIES})
message("CAIRO_LIBS=${CAIRO_LIBS}")
configure_file(
        "src/cairo.pc.in"
        "cairo.pc"
        @ONLY)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cairo.pc"
        DESTINATION "${CMAKE_INSTALL_LIBDIR}/pkgconfig")

