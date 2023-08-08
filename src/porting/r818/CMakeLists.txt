project (r818 C CXX)

set(R818_SRCS ../common/cdlog.cc 
	mediaplayer.c input_linux.cc  )
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

list(APPEND R818_INCLUDE_DIRS ../common)
list(APPEND R818_INCLUDE_DIRS ../include)

if(DIRECTFB_FOUND OR TRUE)
    list(APPEND R818_SRCS ../common/graph_dfb.c)
    list(APPEND R818_INCLUDE_DIRS ${DIRECTFB_INCLUDE_DIRS})
    list(APPEND R818_LIBS ${DIRECTFB_LIBRARIES})
endif()
list(APPEND R818_INCLUDE_DIRS ./inc)

link_directories(${PROJECT_SOURCE_DIR})
list(APPEND R818_LIBS pthread)
include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${R818_INCLUDE_DIRS}
)
set(PREFIX ${CMAKE_INSTALL_PREFIX})

configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
add_library(tvhal SHARED ${R818_SRCS} )
target_link_libraries(tvhal ${R818_LIBS})
install (TARGETS tvhal  DESTINATION lib)
install (FILES ${MILIBS} DESTINATION lib)
