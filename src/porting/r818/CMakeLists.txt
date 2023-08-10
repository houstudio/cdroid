project (r818 C CXX)

set(R818_SRCS ../common/cdlog.cc 
	mediaplayer.c input_linux.cc  )
#find_package(DirectFB)
find_package(SDL2)
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
#message(FATAL_ERROR "SDL2_INCLUDE_DIRS=${SDL2_INCLUDE_DIRS} SDL2_LIBRARIES=${SDL2_LIBRARIES}")
list(APPEND R818_INCLUDE_DIRS ../common)
list(APPEND R818_INCLUDE_DIRS ../include)
if(FALSE)
    list(APPEND R818_SRCS ./graph_fb.c)
elseif(FALSE)
    list(APPEND R818_SRCS ./graph_sdl.c)
    list(APPEND R818_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
    list(APPEND R818_LIBS ${SDL2_LIBRARIES})
elseif(DIRECTFB_FOUND OR TRUE)
    list(APPEND R818_SRCS ../common/graph_dfb.c)
    list(APPEND R818_INCLUDE_DIRS ./directfb/include)#${DIRECTFB_INCLUDE_DIRS})
    list(APPEND R818_LIBS direct directfb)#${DIRECTFB_LIBRARIES})
endif()
list(APPEND R818_INCLUDE_DIRS ./inc)

link_directories(${PROJECT_SOURCE_DIR} ./directfb/lib)
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
target_link_libraries(tvhal ${R818_LIBS} dl)
install (TARGETS tvhal  DESTINATION lib)
install (FILES ${MILIBS} DESTINATION lib)
