
option(MINIMAL_SIZE_OPTIMIZED "For IOT/Embedded size optimize" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_APPS "Build cdroid custom apps" ON)
option(BUILD_CDROID_TESTS "Build unit tests" ON)

option(ENABLE_GIF "enable gif encode and decoder" OFF)
option(ENABLE_JPEG "enable jpeg encode and decoder" OFF)
option(ENABLE_CAIROSVG "enable svg decoder" OFF)
option(ENABLE_MBEDTLS "enable mbedtls" ON)
option(ENABLE_PLPLOT "Enable PLPLot" OFF)
option(ENABLE_DTV "DTV modules support" OFF)
option(FT_WITH_HARFBUZZ "Improve auto-hinting of OpenType fonts." ON)

option(ENABLE_PINYIN2HZ "Chinese Pinyin to HZ support" ON)

set(CMAKE_USE_OPENSSL ON)
set(ENABLE_IPV6 OFF)#for CURL

if(ENABLE_GIF)
   list(APPEND OPTIONAL_LIBS gif)
endif()

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)
find_package(TurboJPEG)
find_package(ZLIB REQUIRED)
find_package(ZIP REQUIRED)
find_package(Freetype2 REQUIRED)
find_package(EXPAT REQUIRED)
find_package(Pixman REQUIRED)
find_package(Cairo REQUIRED)
find_package(MBEDTLS)
#find_package(OpenSSL)
find_package(Fontconfig REQUIRED)
find_package(Brotli)
find_package(BZip2)
find_package(UniBreak REQUIRED)
find_package(litehtml CONFIG)
find_package(PLPLOT)
find_package(zint CONFIG) #barcode generater

list(APPEND CDROID_DEPLIBS
    ${CAIRO_LIBRARIES}
    ${PIXMAN_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE2_LIBRARIES}
    ${ZIP_LIBRARIES}
    ${PNG_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UNIBREAK_LIBRARIES}
)
if ( BROTLIDEC_FOUND )
   #list(APPEND CDROID_DEPLIBS ${BROTLIDEC_LIBRARIES})
endif()
if (BZIP2_FOUND)
   list(APPEND CDROID_DEPLIBS ${BZIP2_LIBRARIES})
endif()
if (TURBOJPEG_FOUND)
   add_definitions(-DENABLE_TURBOJPEG=1)
   list(APPEND CDROID_DEPLIBS ${TURBOJPEG_LIBRARIES})
   list(APPEND CDROID_DEPINCLUDES ${TURBOJPEG_INCLUDE_DIRS})
endif()
if(JPEG_FOUND)
   add_definitions(-DENABLE_JPEG=1)
   list(APPEND CDROID_DEPLIBS ${JPEG_LIBRARIES})
   list(APPEND CDROID_DEPINCLUDES ${JPEG_INCLUDE_DIRSS})
endif()

if (litehtml_FOUND)
    list( APPEND CDROID_DEPLIBS litehtml)
    #list(APPEND CDROID_DEPINCLUDES ${LITEHTML_INCLUDE_DIRS})
    add_definitions(-DENABLE_LITEHTML=1)
endif()

if (PLPLOT_FOUND)
    list( APPEND CDROID_DEPLIBS ${PLPLOT_LIBRARIES})
    list(APPEND CDROID_DEPINCLUDES ${PLPLOT_INCLUDE_DIRS})
    add_definitions(-DENABLE_PLPLOT=1)
endif()

if (zint_FOUND)
    list( APPEND CDROID_DEPLIBS zint::zint)
    #list(APPEND CDROID_DEPINCLUDES ${ZINT_INCLUDE_DIRS})
    add_definitions(-DENABLE_BARCODE=1)
endif()

if(ENABLE_FRIBIDI)
  find_package(Fribidi REQUIRED)
  list(APPEND CDROID_DEPLIBS ${FRIBIDI_LIBRARIES})
endif(ENABLE_FRIBIDI)

list(APPEND CDROID_DEPINCLUDES
    ${PNG_INCLUDE_DIRS}
    ${ZIP_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}/cairo
    ${PIXMAN_INCLUDE_DIRS}
    ${FRIBIDI_INCLUDE_DIRS}
)
if(BROTLI_FOUND)
   list(APPEND CDROID_DEPLIBS ${BROTLIDEC_LIBRARIES})
   list(APPEND CDROID_DEPINCLUDES ${BROTLI_INCLUDE_DIRS})
endif(BROTLI_FOUND)

if(OPENSSL_FOUND)
    list(APPEND CDROID_DEPINCLUDES ${OPENSSL_INCLUDE_DIRS})
    list(APPEND CDROID_DEPLIBS ${OPENSSL_LIBRARIES})
elseif(MBEDTLS_FOUND)
    list(APPEND CDROID_DEPINCLUDES ${MBEDTLS_INCLUDE_DIRS})
    list(APPEND CDROID_DEPLIBS ${MBEDTLS_LIBRARIES})
endif(OPENSSL_FOUND)
message("CDROID_DEPLIBS=${CDROID_DEPLIBS}")

if(ENABLE_PINYIN2HZ)
  list(APPEND OPTIONAL_LIBS pinyin)
  list(APPEND CDROID_DEPLIBS pinyin)
  list(APPEND CDROID_DEPINCLUDES ${CMAKE_SOURCE_DIR}/src/3rdparty/pinyin/include)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/src/gui/gui_features.h.cmake")
   configure_file(src/gui/gui_features.h.cmake  ${CMAKE_BINARY_DIR}/include/gui/gui_features.h)
endif()
set(SKIP_INSTALL_EXPORT TRUE)

if(NOT VCPKG_TARGET_TRIPLET)
foreach(lib ${CDROID_DEPLIBS})
    get_filename_component(libpath ${lib} DIRECTORY)
    set(linkdone FALSE)
    while(NOT linkdone AND EXISTS ${lib})
       execute_process( COMMAND readlink ${lib} OUTPUT_VARIABLE linkfile  OUTPUT_STRIP_TRAILING_WHITESPACE)
       get_filename_component(libname ${lib} NAME)
       get_filename_component(linkpath "${linkfile}" DIRECTORY)
       get_filename_component(linkname "${linkfile}" NAME)
       if("${linkpath}" STREQUAL "")
	   set(linkpath ${libpath})
       endif()
       if("${linkfile}" STREQUAL "")
           install(FILES ${lib} DESTINATION lib)
       else()
	   get_filename_component(fromfile ${linkfile} NAME)
	   install(CODE "execute_process(COMMAND ${CMAKE_COMMAND} -E create_symlink
	       ${fromfile} ${libname} WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/lib)")
       endif("${linkfile" STREQUAL "")
       set(lib ${linkpath}/${linkname})
       if("${linkfile}" STREQUAL "")
	   set(linkdone TRUE)
       endif()
    endwhile()
endforeach(lib ${CDROID_DEPLIBS})
endif(NOT VCPKG_TARGET_TRIPLET)
