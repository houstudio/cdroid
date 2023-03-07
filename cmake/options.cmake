
option(MINIMAL_SIZE_OPTIMIZED "For IOT/Embedded size optimize" OFF)
option(BUILD_EXAMPLES "Build examples" ON)
option(BUILD_CDROID_TESTS "Build unit tests" ON)

option(ENABLE_GIF "enable gif encode and decoder" OFF)
option(ENABLE_CAIROSVG "enable svg decoder" OFF)
option(ENABLE_MBEDTLS "enable mbedtls" ON)
option(ENABLE_PLPLOT "Enable PLPLot" OFF)
option(ENABLE_DTV "DTV modules support" OFF)
option(FT_WITH_HARFBUZZ "Improve auto-hinting of OpenType fonts." ON)

option(ENABLE_PINYIN2HZ "Chinese Pinyin to HZ support" OFF)

set(CMAKE_USE_OPENSSL ON)
set(ENABLE_IPV6 OFF)#for CURL

if(ENABLE_GIF)
   list(APPEND OPTIONAL_LIBS gif)
endif()

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

find_package(ZLIB REQUIRED)
find_package(JSONCPP REQUIRED)
find_package(ZIP REQUIRED)
find_package(Freetype2 REQUIRED)
find_package(EXPAT REQUIRED)
find_package(Pixman REQUIRED)
find_package(Cairo REQUIRED)
find_package(OpenSSL)
find_package(Fontconfig REQUIRED)
find_package(Brotli)
find_package(BZip2 REQUIRED)
find_package(UniBreak REQUIRED)
find_package(LiteHtml)
list(APPEND CDROID_DEPLIBS
    ${CAIRO_LIBRARIES}
    ${PIXMAN_LIBRARIES}
    ${FONTCONFIG_LIBRARIES}
    ${FREETYPE2_LIBRARIES}
    ${BZIP2_LIBRARIES}
    ${ZIP_LIBRARIES}
    ${PNG_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${ZLIB_LIBRARIES}
    ${UNIBREAK_LIBRARIES}
)

if (LITEHTML_FOUND)
    list( APPEND CDROID_DEPLIBS ${LITEHTML_LIBRARIES})
    list(APPEND CDROID_DEPINCLUDES ${LITEHTML_INCLUDE_DIRS})
endif()

if(ENABLE_FRIBIDI)
  find_package(Fribidi REQUIRED)
  list(APPEND CDROID_DEPLIBS ${FRIBIDI_LIBRARIES})
endif(ENABLE_FRIBIDI)

if(ENABLE_TURBOJPEG)
   find_package(TurboJPEG REQUIRED)
   list(APPEND CDROID_DEPLIBS ${TURBOJPEG_LIBRARIES})
endif(ENABLE_TURBOJPEG)

list(APPEND CDROID_DEPINCLUDES
    ${PNG_INCLUDE_DIRS}
    ${JPEG_INCLUDE_DIRSS}
    ${ZIP_INCLUDE_DIRS}
    ${EXPAT_INCLUDE_DIRS}
    ${JSONCPP_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}/cairo
    ${PIXMAN_INCLUDE_DIRS}
    ${FRIBIDI_INCLUDE_DIRS}
    ${TURBOJPEG_INCLUDE_DIRS}
)
if(BROTLI_FOUND)
   list(APPEND CDROID_DEPLIBS ${BROTLIDEC_LIBRARIES})
   list(APPEND CDROID_DEPINCLUDES ${BROTLI_INCLUDE_DIRS})
endif(BROTLI_FOUND)
message("CDROID_DEPLIBS=${CDROID_DEPLIBS}")
if(OPENSSL_FOUND)
    list(APPEND CDROID_DEPINCLUDES ${OPENSSL_INCLUDE_DIRS})
    list(APPEND CDROID_DEPLIBS ${OPENSSL_LIBRARIES})
endif(OPENSSL_FOUND)

if(ENABLE_PINYIN2HZ)
  list(APPEND OPTIONAL_LIBS pinyin)
endif()

if(EXISTS "${CMAKE_SOURCE_DIR}/src/gui/gui_features.h.cmake")
   configure_file(src/gui/gui_features.h.cmake  ${CMAKE_BINARY_DIR}/include/gui/gui_features.h)
endif()
set(SKIP_INSTALL_EXPORT TRUE)
