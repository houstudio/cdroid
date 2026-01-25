project (rk1126 C CXX)

set(RK1126_SRCS ../common/cdlog.cc
        ../common/input_linux.cc
	mediaplayer.c misc.c disp.c)

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

list(APPEND RK1126_INCLUDE_DIRS ../common)
list(APPEND RK1126_INCLUDE_DIRS ../include)

list(APPEND RK1126_LIBS pthread dl)
list(APPEND RKTARGET tvhal)
find_package(DRM REQUIRED)

find_package(PkgConfig)
pkg_check_modules(PC_RGA librga)
find_path(RGA_INCLUDE_DIRS
    NAMES rga/RgaApi.h
    HINTS ${PC_RGA_INCLUDEDIR}
    ${PC_RGA_INCLUDE_DIRS}
    #PATH_SUFFIXES rga
)

find_library(RGA_LIBRARIES
    NAMES rga
    HINTS ${PC_RGA_LIBDIR}
    ${PC_RGA_LIBRARY_DIRS}
)

include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${DRM_INCLUDE_DIRS}
)
set(PREFIX ${CMAKE_INSTALL_PREFIX})
#set(NONPKG_LIBS "-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_gfx \
# -lmi_disp -lmi_ai -lmi_ao -lmi_vdec -lmi_divp -lmi_panel \
# -lhvplayer -lavformat -lavcodec -lavutil -lswresample -lswscale")

add_library(tvhal SHARED ${RK1126_SRCS} ../common/graph_fb.c)
target_link_libraries(tvhal ${RK1126_LIBS})

add_library(tvhal-drm SHARED ${RK1126_SRCS} ../common/graph_drm.cc)
target_link_libraries(tvhal-drm ${RK1126_LIBS} ${DRM_LIBRARIES})
add_dependencies(tvhal tvhal-drm)

file(GLOB MILIBS ${PROJECT_SOURCE_DIR}/libs/*.so)
install (TARGETS ${RKTARGET}  DESTINATION lib)
