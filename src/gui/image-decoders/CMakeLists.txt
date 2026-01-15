project(image-decoders C CXX)
find_package(PNG REQUIRED)
find_package(LCMS2)
find_package(Fontconfig)
set(DECODER_SRCS
    imagedecoder.cc
    bmpdecoder.cc
    gifdecoder.cc
    pngdecoder.cc
    jpegdecoder.cc
    jpeg2000decoder.cc
    webpdecoder.cc
    framesequence.cc
    gifframesequence.cc
    pngframesequence.cc
    webpframesequence.cc
)

include_directories(
    ./ ../
    ../../porting/include
    ${FREETYPE2_INCLUDE_DIRS}
    ${CAIRO_INCLUDE_DIRS}
    ${WEBP_INCLUDE_DIRS}
    ${OPENJPEG_INCLUDE_DIRS}
    ${FONTCONFIG_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_BINARY_DIR}/include/gui
)

list(APPEND IMAGE_DEPLIBS ${PNG_LIBRARIES})

if(ENABLE_JPEG)
   list(APPEND IMAGE_DEPLIBS ${JPEG_LIBRARIES})
endif()

if(ENABLE_TURBOJPEG)
    list(APPEND IMAGE_DEPLIBS ${TURBOJPEG_LIBRARIES})
endif()

if(ENABLE_GIF)
   list(APPEND IMAGE_DEPLIBS GIF::GIF)
endif()

if(ENABLE_WEBP)
   list(APPEND IMAGE_DEPLIBS ${WEBP_LIBRARIES})
endif()

if(ENABLE_OPENJPEG)
   list(APPEND IMAGE_DEPLIBS ${OPENJPEG_LIBRARIES})
endif()
if(ENABLE_LCMS)
    list(APPEND IMAGE_DEPLIBS ${LCMS2_LIBRARIES})
    include_directories(${LCMS2_INCLUDE_DIRS})
endif()

add_library(image-decoders STATIC ${DECODER_SRCS})
target_link_libraries(image-decoders ${IMAGE_DEPLIBS})

add_executable(imagetest image-test.cc)
target_link_libraries(imagetest tvhal cdroid ${IMAGE_DEPLIBS} ${JPEG_LIBRARIES} image-decoders)
