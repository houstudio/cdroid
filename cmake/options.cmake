
option(MINIMAL_SIZE_OPTIMIZED "For IOT/Embedded size optimize" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_CDROID_TESTS "Build unit tests" ON)

option(ENABLE_GIF "enable gif encode and decoder" OFF)
option(ENABLE_CAIROSVG "enable svg decoder" OFF)
option(ENABLE_MBEDTLS "enable mbedtls" ON)
option(ENABLE_UPNP "enable upnp/dlna" OFF)
option(ENABLE_GESTURE "enable gestrure" OFF)
option(ENABLE_PLPLOT "Enable PLPLot" OFF)
option(ENABLE_DTV "DTV modules support" OFF)
option(WITH_JPEG8 "Emulate libjpeg v8 API/ABI (this makes ${CMAKE_PROJECT_NAME} backward-incompatible with libjpeg v6b)" ON)
option(FT_WITH_HARFBUZZ "Improve auto-hinting of OpenType fonts." ON)

option(ENABLE_PINYIN2HZ "Chinese Pinyin to HZ support" OFF)

set(CMAKE_USE_OPENSSL ON)
set(ENABLE_IPV6 OFF)#for CURL

if(ENABLE_GIF)
   list(APPEND OPTIONAL_LIBS gif)
endif()

find_package(PNG REQUIRED)
find_package(JPEG REQUIRED)

if(ENABLE_TURBOJPEG)
   find_package(TurboJPEG REQUIRED)
   list(APPEND CDROID_DEPLIBS ${TURBOJPEG_LIBRARIES})
endif(ENABLE_TURBOJPEG)

find_package(JSONCPP REQUIRED)
find_package(ZIP REQUIRED)
find_package(Freetype2 REQUIRED)
find_package(EXPAT REQUIRED)
find_package(Pixman REQUIRED)
find_package(Cairo REQUIRED)
find_package(OpenSSL)
#find_package(unibreak REQUIRED)

if(ENABLE_FRIBIDI)
  find_package(Fribidi REQUIRED)
  list(APPEND CDROID_DEPLIBS ${FRIBIDI_LIBRARIES})
endif(ENABLE_FRIBIDI)

list(APPEND CDROID_DEPLIBS 
    ${PNG_LIBRARIES}
    ${JPEG_LIBRARIES}
    ${ZIP_LIBRARIES}
    ${EXPAT_LIBRARIES}
    ${JSONCPP_LIBRARIES}
    ${CAIRO_LIBRARIES}
    ${PIXMAN_LIBRARIES}
)

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
