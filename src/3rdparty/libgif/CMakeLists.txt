project(gif C CXX)

set(SRCS_GIF 
    dgif_lib.c egif_lib.c 
    gifalloc.c  gif_err.c 
    gif_hash.c quantize.c)

include_directories(./)

add_library(gif STATIC ${SRCS_GIF})
set_target_properties(gif PROPERTIES POSITION_INDEPENDENT_CODE 1)
file(COPY gif_lib.h DESTINATION ${CMAKE_BINARY_DIR}/include/gif)
