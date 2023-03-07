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
find_package(SDL2 REQUIRED)

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
    list(APPEND X64_LIBRARIES ${DRM_LIBRARY})
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

if (SDL2_FOUND)
    list(APPEND X64_INCLUDE_DIRS ${SDL2_INCLUDE_DIRS})
    list(APPEND X64_LIBRARIES ${SDL2_LIBRARIES})
    list(APPEND X64_SRCS graph_sdl.c)
endif()

find_package(X11)
find_package(PkgConfig REQUIRED)
if( X11_FOUND AND NOT SDL2_FOUND)
    list(APPEND X64_SRCS graph_xlib.c)
    list(APPEND X64_LIBRARIES ${X11_LIBRARIES})
    list(APPEND X64_INCLUDE_DIRS ${X11_INCLUDE_DIR})
endif()
if(NOT X64_LIBRARIES)
    list(APPEND X64_SRCS graph_rfb.c)
    list(APPEND X64LIBS vncserver)
endif()

list(APPEND X64)LIBRARIES pthread)
find_package(ZLIB)
include_directories(./
    ../common ../include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${ZLIB_INCLUDE_DIR}
    ${X64_INCLUDE_DIRS}
)
message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} X64_SRCS=${X64_SRCS}")
add_library(tvhal SHARED ${X64_SRCS} )
target_link_libraries(tvhal PRIVATE ${X64_LIBRARIES})

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(NONPKG_LIBS "-L${CMAKE_INSTALL_PREFIX}/lib")
configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
install (TARGETS tvhal  DESTINATION lib)
