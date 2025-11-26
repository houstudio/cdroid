project (rk3506 C CXX)

set(RK3506_SRCS
	../common/cdlog.cc
	../common/input_linux.cc
	)
include(CheckIncludeFile)

check_include_file(poll.h HAVE_POLL_H)
check_include_file(sys/epoll.h  HAVE_EPOLL_H)
check_include_file(linux/input.h HAVE_INPUT_H)
check_include_file(execinfo.h HAVE_EXECINFO_H)
check_include_file(drm/drm.h HAVE_DRM_H)
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

list(APPEND RK3506_INCLUDE_DIRS ../common)
list(APPEND RK3506_INCLUDE_DIRS ../include)
find_package(DRM)
include_directories(./
    ../common
    ../include
    ./include
    ${DRM_INCLUDE_DIRS}
    ${PIXMAN_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${RK3506_INCLUDE_DIRS}
)
set(PREFIX ${CMAKE_INSTALL_PREFIX})

add_library(tvhal-fb SHARED ${RK3506_SRCS} ../common/graph_fb.c)
add_library(tvhal-drm SHARED ${RK3506_SRCS} ../common/graph_drm.cc)
add_library(tvhal SHARED ${RK3506_SRCS} ../common/graph_fb.c)
target_link_libraries(tvhal ${RK3506_LIBS} dl)
install (TARGETS tvhal DESTINATION lib)
