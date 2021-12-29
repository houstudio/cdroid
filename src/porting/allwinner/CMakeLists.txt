project (allwinner C CXX)

set(ALLWINNER_SRCS 
     input_linux.cc
     graph_nx5fb.c
    )
include(CheckIncludeFile)
check_include_file(poll.h HAVE_POLL_H)
check_include_file(sys/epoll.h  HAVE_EPOLL_H)
check_include_file(linux/input.h HAVE_INPUT_H)
check_include_file(execinfo.h HAVE_EXECINFO_H)
if(HAVE_POLL_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_POLL_H)
endif()
if(HAVE_EPOLL_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_EPOLL_H)
endif()
if(HAVE_INPUT_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_INPUT_H)
endif()
if(HAVE_EXECINFO_H)
    list(APPEND CMAKE_REQUIRED_DEFINITIONS -DHAVE_EXECINFO_H)
endif()

set(NX5ROOT_DIR)
find_library(LIBVIDEODEC_PATH VideoDec ${NX5ROOT_DIR}/thirdparty/lib/codec)

if(LIBVIDEODEC_PATH)
    add_definitions(-DHAVE_FY_TDE2=1)
endif()

#sudo apt install libx11-dev
if( ENABLE_RFB )
    add_definitions(-DENABLE_RFB=1)
    list(APPEND ALLWINNER_LIBS vncserver)
endif()

list(APPEND ALLWINNER_LIBS pthread ${LIBVIDEODEC_PATH})

include_directories(./
    ../include
    ${NX5ROOT_DIR}/thirdparty/include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libvncserver
    ${CMAKE_BINARY_DIR}/src/3rdparty/libvncserver
    ${CMAKE_SOURCE_DIR}/deps/include
    ${CMAKE_SOURCE_DIR}/deps/include/gui #for eventcodes.h
)

add_library(allwinner STATIC ${ALLWINNER_SRCS} )
target_link_libraries(allwinner ${ALLWINNER_LIBS})

