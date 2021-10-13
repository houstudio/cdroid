project (x86 C CXX)

set(X86_SRCS 
     ngl_panel.c
     ngl_mediaplayer.c
     input_linux.cc
    )

if (ENABLE_DTV)
    list(APPEND X86_SRCS  ngl_dsc.c ngl_smc.c  ngl_tuner.c ngl_dmx.cc 
          ngl_nvm.c  ngl_pvr.c ngl_snd.c ngl_video.c dmxreader.cc
          ngl_disp.c ngl_os.c 
    )
endif()
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
    list(APPEND X86LIBS vncserver)
endif()

string(TOLOWER ${CMAKE_SYSTEM_NAME} SYSNAME_LOWER)
if ( SYSNAME_LOWER MATCHES "linux" OR SYSNAME_LOWER MATCHES "cygwin" )
    find_package(X11)
    find_package(PkgConfig REQUIRED)
	pkg_check_modules(GTK3 gtk+-3.0)
    message("X11_INCLUDE_DIR=${X11_INCLUDE_DIR} ${X11_FOUND} ${GTK3_LIBRARIES} ${GTK3_FOUND}")
    if( X11_FOUND )
        list(APPEND X86_SRCS graph_xlib.c)
        list(APPEND X86LIBS ${X11_LIBRARIES})
    elseif( GTK3_FOUND )
        list(APPEND X86_SRCS graph_gtk.c)
        list(APPEND X86LIBS ${GTK3_LIBRARIES})
    else()
        list(APPEND X86_SRCS graph_rfb.c)
        list(APPEND X86LIBS vncserver)
    endif()
elseif (WIN32)
    list(APPEND X86_SRCS graph_win32.c)
    list(APPEND X86LIBS win32.lib)
endif()

list(APPEND X86LIBS pthread)

include_directories(./
    ../include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${X11_INCLUDE_DIRS}
    ${GTK3_INCLUDE_DIRS}
    ${CMAKE_BINARY_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/3rdparty/libvncserver
    ${CMAKE_BINARY_DIR}/src/3rdparty/libvncserver
    ${CMAKE_SOURCE_DIR}/deps/include
    ${CMAKE_SOURCE_DIR}/deps/include/gui #for eventcodes.h
)

add_library(x86 STATIC ${X86_SRCS} )
target_link_libraries(x86 ${X86LIBS})

