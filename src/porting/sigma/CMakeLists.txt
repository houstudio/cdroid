project (sigma C CXX)

set(SIGMA_SRCS input_linux.cc  )

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

#sudo apt install libx11-dev
if( ENABLE_RFB )
    add_definitions(-DENABLE_RFB=1)
    list(APPEND SIGMA_SRCS ../common/rfbcommon.cc)
    list(APPEND SIGMA_LIBS vncserver)
    find_package(ZLIB)
endif()

if(TRUE)
    list(APPEND SIGMA_SRCS graph_fb.c)
else()
    list(APPEND SIGMA_SRCS graph_rfb.c)
    list(APPEND SIGMA_LIBS vncserver)
endif()

link_directories(${PROJECT_SOURCE_DIR}/libs)
list(APPEND SIGMA_LIBS pthread 
	-L${PROJECT_SOURCE_DIR}/libs 
	-lmi_common -lmi_sys -lmi_gfx)
find_package(ZLIB)
include_directories(./
    ./inc ../include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${ZLIB_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libvncserver
    ${CMAKE_BINARY_DIR}/src/3rdparty/libvncserver
)

add_library(sigma STATIC ${SIGMA_SRCS} )
target_link_libraries(sigma ${SIGMA_LIBS})

