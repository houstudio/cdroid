project (x86 C CXX)

set(X64_SRCS 
     ../common/cdlog.cc
     ngl_panel.c
     ngl_mediaplayer.c
     input_linux.cc
    )

if (ENABLE_DTV)
    list(APPEND X64_SRCS  ngl_dsc.c ngl_smc.c  ngl_tuner.c ngl_dmx.cc 
          ngl_nvm.c  ngl_pvr.c ngl_snd.c ngl_video.c dmxreader.cc
          ngl_disp.c ngl_os.c 
    )
endif()
include(CheckIncludeFile)
check_include_file(poll.h HAVE_POLL_H)
check_include_file(sys/epoll.h  HAVE_EPOLL_H)
check_include_file(linux/input.h HAVE_INPUT_H)
check_include_file(execinfo.h HAVE_EXECINFO_H)
check_include_file(drm.h HAVE_DRM_H)
find_package(PkgConfig)
pkg_check_modules(PC_DRM dri)

find_path(
    DRM_INCLUDE_DIR
    NAMES drm.h
    HINTS ${PC_DRM_INCLUDEDIR}
    ${PC_DRM_INCLUDE_DIRS}
    DOC "Include directory for the drm library")

find_library(DRM_LIBRARY
    NAMES drm
    HINTS ${PC_DRM_LIBDIR}
    ${PC_DRM_LIBRARY_DIRS}
)

if(DRM_LIBRARY AND DRM_INCLUDE_DIR)
    list(APPEND X64LIBS ${DRM_LIBRARY})
endif()

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
    list(APPEND X64_SRCS ../common/rfbcommon.cc)
    list(APPEND X64LIBS vncserver)
    find_package(ZLIB)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSNAME_LOWER)
if ( SYSNAME_LOWER MATCHES "linux" OR SYSNAME_LOWER MATCHES "cygwin" )
    find_package(X11)
    find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 gtk+-3.0)
    message("X11_INCLUDE_DIR=${X11_INCLUDE_DIR} ${X11_FOUND} ${GTK3_LIBRARIES} ${GTK3_FOUND} ${}")
    if (FALSE AND DRM_LIBRARY)
	list(APPEND X64_SRCS gfxdrm.cc graph_drm.cc)
	list(APPEND X64LIBS ${DRM_LIBRARY})
    elseif(FALSE)
        list(APPEND X64_SRCS graph_fb.c)
    elseif( X11_FOUND )
        list(APPEND X64_SRCS graph_xlib.c)
        list(APPEND X64LIBS ${X11_LIBRARIES})
    elseif( GTK3_FOUND )
        list(APPEND X64_SRCS graph_gtk.c)
        list(APPEND X64LIBS ${GTK3_LIBRARIES} ${DRM_LIBRARY})
    else()
        list(APPEND X64_SRCS graph_rfb.c)
        list(APPEND X64LIBS vncserver)
    endif()
elseif (WIN32)
    list(APPEND X64_SRCS graph_win32.c)
    list(APPEND X64LIBS win32.lib)
endif()

list(APPEND X64LIBS pthread)
find_package(ZLIB)
include_directories(./
    ../common ../include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${DRM_INCLUDE_DIR}
    ${X11_INCLUDE_DIRS}
    ${ZLIB_INCLUDE_DIR}
)
message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR}")
add_library(tvhal SHARED ${X64_SRCS} )
target_link_libraries(tvhal PRIVATE ${X64LIBS})
install (TARGETS tvhal  DESTINATION lib)
