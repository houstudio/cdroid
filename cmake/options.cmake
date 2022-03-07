
option(MINIMAL_SIZE_OPTIMIZED "For IOT/Embedded size optimize" OFF)
option(BUILD_DOCS "Build documents" OFF)
option(BUILD_EXAMPLES "Build examples" OFF)
option(BUILD_CDROID_TESTS "Build unit tests" ON)

option(ENABLE_GIF "enable gif encode and decoder" OFF)
option(ENABLE_CAIROSVG "enable svg decoder" OFF)
option(ENABLE_MBEDTLS "enable mbedtls" OFF)
option(ENABLE_UPNP "enable upnp/dlna" OFF)
option(ENABLE_GESTURE "enable gestrure" OFF)
option(ENABLE_TURBOJPEG "enable turbo jpeg" ON)
option(ENABLE_MP3ID3 "enable mp3 ids" ON)
option(ENABLE_FRIBIDI "BIDI Text Layout  support" ON)
option(ENABLE_PLPLOT "Enable PLPLot" OFF)
option(ENABLE_DTV "DTV modules support" OFF)
option(WITH_JPEG8 "Emulate libjpeg v8 API/ABI (this makes ${CMAKE_PROJECT_NAME} backward-incompatible with libjpeg v6b)" ON)
option(FT_WITH_HARFBUZZ "Improve auto-hinting of OpenType fonts." ON)

option(ENABLE_PINYIN2HZ "Chinese Pinyin to HZ support" OFF)

add_definitions(-DPNG_ZLIB_VERNUM=0)#Make PNG compiled happy:)

if( MINIMAL_SIZE_OPTIMIZED )
    set(ENABLE_GIF OFF)
    set(ENABLE_CAIROSVG OFF)
    set(ENABLE_TURBOJPEG OFF)
    set(ENABLE_JPEG OFF)
    set(ENABLE_GESTURE OFF)
    set(ENABLE_FRIBIDI OFF)
    set(ENABLE_CAIROSVG OFF)
    set(ENABLE_MBEDTLS OFF)
    set(FT_WITH_HARFBUZZ OFF)
    set(ENABLE_UPNP OFF)
    set(ENABLE_MP3ID3 OFF)
    set(ENABLE_PINYIN2HZ OFF)
    set(ENABLE_PLPLOT OFF)
endif( MINIMAL_SIZE_OPTIMIZED )

if(ENABLE_GIF)
  list(APPEND OPTIONAL_LIBS gif)
endif()

if(ENABLE_CAIROSVG)
  list(APPEND OPTIONAL_LIBS svg-cairo)
endif()

if(ENABLE_MBEDTLS)
endif()

if(ENABLE_UPNP)
  list(APPEND APP_EXTLIBS upnp)
endif()

if(ENABLE_GESTURE)
  list(APPEND OPTIONAL_LIBS grt)
endif()

if(ENABLE_TURBOJPEG)
  list(APPEND OPTIONAL_LIBS turbojpeg)
endif()

if(ENABLE_MP3ID3)
  list(APPEND APP_EXTLIBS id3)
endif()

if(ENABLE_FRIBIDI)
  list(APPEND OPTIONAL_LIBS fribidi_static iconv)
endif()

if(ENABLE_PLPLOT)
  list(APPEND OPTIONAL_LIBS plplot plplotcxx)
  set(NaNAwareCCompiler ON)#make plplot compile happy
endif()

if(ENABLE_PINYIN2HZ)
  list(APPEND OPTIONAL_LIBS pinyin)
endif()



if(EXISTS "${CMAKE_SOURCE_DIR}/src/gui/gui_features.h.cmake")
configure_file(src/gui/gui_features.h.cmake  ${CMAKE_BINARY_DIR}/include/gui/gui_features.h)
endif()
set(SKIP_INSTALL_EXPORT TRUE)
