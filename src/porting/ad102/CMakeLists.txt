project (ad102 C CXX)

set(AD102_SRCS
	../common/cdlog.cc
	../common/input_linux.cc
    ./cdplayer.c
	)
include(CheckIncludeFile)

check_include_file(poll.h HAVE_POLL_H)
check_include_file(sys/epoll.h  HAVE_EPOLL_H)
check_include_file(linux/input.h HAVE_INPUT_H)
check_include_file(execinfo.h HAVE_EXECINFO_H)
check_include_file(drm.h HAVE_DRM_H)
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

list(APPEND AD102_INCLUDE_DIRS ../common)
list(APPEND AD102_INCLUDE_DIRS ../include)

include_directories(./
    ${AD102_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${FFMPEG_INCLUDE_DIRS}
)

set(AD102_LIBS
    z ssl crypto
    avcodec avutil avfilter swresample swscale avformat
    asound utils2
    media media_ffmpeg
    hardware2 2d)
add_library(tvhal SHARED ${AD102_SRCS} ../common/graph_fb.c)
add_library(tvhal-g2d SHARED ${AD102_SRCS} ./graph_g2d.c)
target_link_libraries(tvhal ${AD102_LIBS} )
target_link_libraries(tvhal-g2d ${AD102_LIBS})

set_target_properties(tvhal tvhal-g2d PROPERTIES LINK_INTERFACE_MULTIPLICITY 3)

add_dependencies(tvhal tvhal-g2d)
install (TARGETS tvhal tvhal-g2d  DESTINATION lib)
