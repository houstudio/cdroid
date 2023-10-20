project (x86 C CXX)

set(X64_SRCS
     ../common/cdlog.cc
     ../common/input_linux.cc
     cdplayer.c
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

find_package(PkgConfig)
find_package(SDL2)
find_package(DirectFB)
find_package(X11)
find_package(DRM)
if(X11_FOUND)
    include_directories(${X11_INCLUDE_DIR}) 
    add_library(tvhal-x11 SHARED ${X64_SRCS} ../common/graph_xlib.c)
    target_link_libraries(tvhal-x11 ${X11_LIBRARIES})
    list(APPEND X64PORTS tvhal-x11)
endif(X11_FOUND)
if(DRM_FOUND)
    include_directories(${DRM_INCLUDE_DIRS})
    add_library(tvhal-drm SHARED ${X64_SRCS} ../common/graph_drm.cc)
    target_link_libraries(tvhal-drm ${DRM_LIBRARIES})
    list(APPEND X64PORTS tvhal-drm)
endif()
if(DIRECTFB_FOUND)
    include_directories(${DIRECTFB_INCLUDE_DIRS})
    add_library(tvhal-dfb SHARED ${X64_SRCS} ../common/graph_dfb.c)
    target_link_libraries(tvhal-dfb ${DIRECTFB_LIBRARIES})
    list(APPEND X64PORTS tvhal-dfb)
endif(DIRECTFB_FOUND)
if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIRS})
    add_library(tvhal-sdl SHARED ${X64_SRCS} ../common/graph_sdl.c)
    target_link_libraries(tvhal-sdl ${SDL2_LIBRARIES})
    list(APPEND X64PORTS tvhal-sdl)
endif(SDL2_FOUND)
if(NOT X64PORTS)
    message(FATAL_ERROR "graph must implemented")
    find_package(ZLIB)
    add_definitions(-DENABLE_RFB=1)
    list(APPEND GRAPH_SRCS graph_rfb.c ../common/rfbcommon.cc)
    list(APPEND X64_LIBRARIES vncserver)
endif()

include_directories(./
    ../common ../include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${X64_INCLUDE_DIRS}
)
message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} GRAPH_SRCS=${GRAPH_SRCS}")
add_library(tvhal SHARED ${X64_SRCS} ../common/graph_xlib.c)
target_link_libraries(tvhal PRIVATE ${X11_LIBRARIES})

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(NONPKG_LIBS "-L${CMAKE_INSTALL_PREFIX}/lib")
configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
install (TARGETS ${X64PORTS}  DESTINATION lib)

