project(image-decoders C CXX)

find_package(PNG REQUIRED)
find_package(GIF)
find_package(JPEG)
find_package(WEBP)
find_package(LCMS2)

set(DECODER_SRCS
    imagedecoder.cc
    pngdecoder.cc
    jpegdecoder.cc
    framesequence.cc
    gifframesequence.cc
    pngframesequence.cc
    webpframesequence.cc
    #pngchunks.cc
)

include_directories(
	./ ../
	../../porting/include
	${FREETYPE2_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
	${WEBP_INCLUDE_DIRS}
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

if(ENABLE_GIF AND GIF_FOUND)
   list(APPEND IMAGE_DEPLIBS GIF::GIF)
endif()

if(ENABLE_WEBP OR WEBP_FOUND)
   list(APPEND IMAGE_DEPLIBS ${WEBP_LIBRARIES})
endif()

if(LCMS2_FOUND AND ENABLE_LCMS)
    list(APPEND IMAGE_DEPLIBS ${LCMS2_LIBRARIES})
else()
    set(ENABLE_LCMS OFF)
endif()

add_library(image-decoders STATIC ${DECODER_SRCS})
target_link_libraries(image-decoders ${IMAGE_DEPLIBS})

add_executable(imagetest image-test.cc)
target_link_libraries(imagetest tvhal cdroid ${IMAGE_DEPLIBS} ${JPEG_LIBRARIES} image-decoders)
