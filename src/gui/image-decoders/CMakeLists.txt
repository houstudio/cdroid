project(image-decoders C CXX)
find_package(WEBP)
aux_source_directory(. DECODER_SRCS)
include_directories(
	./ ../
	../../porting/include
	${CMAKE_BINARY_DIR}/include
	${FREETYPE2_INCLUDE_DIRS}
	${CAIRO_INCLUDE_DIRS}
	${WEBP_INCLUDE_DIRS})
if(ENABLE_GIF)
   list(APPEND IMAGE_DEPLIBS ${GIF_LIBRARIES})
endif()
if(ENABLE_WEBP)
   list(APPEND IMAGE_DEPLIBS ${WEBP_LIBRARIES})
endif()

add_library(image-decoders STATIC ${DECODER_SRCS})
target_link_libraries(image-decoders ${IMAGE_DEPLIBS})
