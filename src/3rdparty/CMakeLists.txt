ADD_CMAKE_PACKAGE(zlib-1.2.11)
ADD_CMAKE_PACKAGE(libpng)
#ADD_CMAKE_PACKAGE(liblzma)
ADD_CMAKE_PACKAGE(libxml2-v2.9.12)
#ADD_CMAKE_PACKAGE(libxslt-1.1.37)
ADD_CMAKE_PACKAGE(expat)
ADD_CMAKE_PACKAGE(libjpeg-turbo)
ADD_CMAKE_PACKAGE(pixman)

#1... build freetype (shared) without harfbuzz
SET(FT_DISABLE_HARFBUZZ ON)
SET(BUILD_SHARED_LIB ON)
SET(FT_REQUIRE_BROTLI OFF)
ADD_CMAKE_PACKAGE(freetype-2.11.0)

#2...build harfbuzz(static)without fontconfig and cairo
if(FT_WITH_HARFBUZZ)
   ADD_CMAKE_PACKAGE(icu68)
   ADD_CMAKE_PACKAGE(harfbuzz)
   ADD_CMAKE_PACKAGE(graphite2-1.3.14)
endif()

#3...build freetype (static) with harfbuzz
SET(FT_DISABLE_HARFBUZZ OFF)
SET(FT_REQUIRE_HARFBUZZ ON)
BUILD_CMAKE_PACKAGE(freetype-2.11.0)

#4...build fontconfig(static)
ADD_CMAKE_PACKAGE(fontconfig)

#5...build harfbuzz(
find_package(Fontconfig)
ADD_CMAKE_PACKAGE(cairo-1.16.0)
ADD_CMAKE_PACKAGE(libunibreak)
ADD_CMAKE_PACKAGE(libiconv)
ADD_CMAKE_PACKAGE(lzo-2.10)

if(ENABLE_CAIROSVG)
   ADD_CMAKE_PACKAGE(libsvg)
   ADD_CMAKE_PACKAGE(libsvg-cairo)
   ADD_CMAKE_PACKAGE(uriparser)
endif()

ADD_CMAKE_PACKAGE(mbedtls-2.8.0)
ADD_CMAKE_PACKAGE(openssl-1.1.1)
ADD_CMAKE_PACKAGE(curl)

ADD_CMAKE_PACKAGE(gtest)
ADD_CMAKE_PACKAGE(jsoncpp)
ADD_CMAKE_PACKAGE(id3)
ADD_CMAKE_PACKAGE(libgif)
ADD_CMAKE_PACKAGE(libzip)
ADD_CMAKE_PACKAGE(uuid)
ADD_CMAKE_PACKAGE(fribidi)
ADD_CMAKE_PACKAGE(pinyin)

if(ENABLE_PLPLOT)
   ADD_CMAKE_PACKAGE(shapelib)
   ADD_CMAKE_PACKAGE(plplot)
endif()

if(ENABLE_GESTURE)
   ADD_CMAKE_PACKAGE(grt/build)
endif(ENABLE_GESTURE)


ADD_CMAKE_PACKAGE(libvncserver)

ADD_CUSTOM_TARGET(cdroiddeps
    COMMENT "config done all dependency packages!"
    )
ADD_DEPENDENCIES(cdroiddeps  ${THIRDPARTY_BUILD_TARGETS})
