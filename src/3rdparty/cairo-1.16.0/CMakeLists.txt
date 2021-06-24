project(cairo)

set(CMAKE_MODULE_PATH "${CMAKE_BINARY_DIR}/cmake")
find_package(PNG 16)
find_package(ZLIB)
find_package(freetype)
find_package(Threads)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -Wno-attributes ")

if ( NOT ZLIB_FOUND )
set(ZLIB_LIBRARY zlib)
set(ZLIB_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(ZLIB_FOUND TRUE)
endif()

if (NOT PNG_FOUND )
set(PNG_LIBRARY png)
set(PNG_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(CAIRO_HAS_PNG_FUNCTIONS TRUE)
set(PNG_FOUND TRUE)
endif()

set(PIXMAN_LIBRARY pixman-1)
set(PIXMAN_INCLUDE_DIR ${CMAKE_BINARY_DIR}/include)
set(PIXMAN_FOUND TRUE)

set(FONTCONFIG_LIBRARY fontconfig uuid)
set(FONTCONFIG_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/src/3rdparty/fontconfig/)
set(FONTCONFIG_FOUND TRUE)

if(FREETYPE_FOUND)
#    find_package(BZip2)
    include_directories(${FREETYPE_INCLUDE_DIRS})
#    include_directories(${BZIP2_INCLUDE_DIR})
list(APPEND CAIRO_LIBS ${FREETYPE_LIBRARIES} ${BZIP2_LIBRARIES} ${FONTCONFIG_LIBRARY})
else()
    set(FREETYPE_FOUND TRUE)
    include_directories(${CMAKE_BINARY_DIR}/include 
        ${CMAKE_BINARY_DIR}/include/freetype2)
list(APPEND CAIRO_LIBS freetype ${BZIP2_LIBRARIES} ${FONTCONFIG_LIBRARY})
endif()

include(Configure_config.cmake)
include(Configure_features.cmake)

if(CAIRO_HAS_PTHREAD)
    list(APPEND CAIRO_LIBS -lpthread)
endif()

include_directories(
   ${PROJECT_BINARY_DIR}
   ${CMAKE_BINARY_DIR}/include/
   ${CMAKE_BINARY_DIR}/include/freetype2
   ${CMAKE_BINARY_DIR}/include/porting
   ${CMAKE_BINARY_DIR}/include/pixman
)
include_directories(${FONTCONFIG_INCLUDE_DIR})

if (WIN32)
    set(CAIRO_LIBS gdi32 msimg32 user32 winmm)
endif()

include_directories(${PIXMAN_INCLUDE_DIR} ${PNG_INCLUDE_DIR} ${ZLIB_INCLUDE_DIR})
list(APPEND CAIRO_LIBS  ${PIXMAN_LIBRARY} ${PNG_LIBRARY} ${ZLIB_LIBRARY})

if(FONTCONFIG_FOUND)
    list(APPEND CAIRO_LIBS ${FONTCONFIG_LIBRARY})
endif()

include_directories(src)
add_subdirectory(src)
