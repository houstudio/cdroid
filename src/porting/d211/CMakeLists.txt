project (d211 C CXX)

set(D211_SRCS ../common/cdlog.cc
        ../common/input_linux.cc
	mediaplayer.c
	)
find_package(DirectFB)
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

list(APPEND D211_INCLUDE_DIRS ../common)
list(APPEND D211_INCLUDE_DIRS ../include ./include ./ge ./ge/include )

if(DIRECTFB_FOUND)
    list(APPEND D211_SRCS ../common/graph_dfb.c)
    list(APPEND D211_INCLUDE_DIRS ${DIRECTFB_INCLUDE_DIRS})
    list(APPEND D211_LIBS ${DIRECTFB_LIBRARIES})
else()    
    list(APPEND D211_SRCS graph_gfx.c)
endif()
list(APPEND D211_INCLUDE_DIRS ./inc)
list(APPEND D211_LIBS pthread dl)

include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${D211_INCLUDE_DIRS}
)
set(PREFIX ${CMAKE_INSTALL_PREFIX})

configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
add_library(tvhal SHARED ${D211_SRCS} )
target_link_libraries(tvhal ${D211_LIBS})
file(GLOB MILIBS ${PROJECT_SOURCE_DIR}/libs/*.so)
install (TARGETS tvhal  DESTINATION lib)
install (FILES ${MILIBS} DESTINATION lib)
