project (x86 C CXX)

set(X64_SRCS
     ../common/cdlog.cc
     ../common/input_linux.cc
     cdplayer.c misc.c
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
find_package(XCB REQUIRED)
find_package(XCB_IMAGE)
find_package(DRM)
find_package(Pixman)
find_package(OpenGL)

find_package(PkgConfig REQUIRED)
find_path(XCB_IMAGE_INCLUDE_DIR NAMES xcb/xcb_image.h)
find_library(XCB_IMAGE_LIBRARY NAMES xcb-image)

include_directories(../common ../include ${PIXMAN_INCLUDE_DIRS})

add_library(tvhal-fb SHARED ${X64_SRCS} ../common/graph_fb.c)
target_link_libraries(tvhal-fb)
list(APPEND X64PORTS tvhal-fb)

if(OPENGL_FOUND AND X11_FOUND)
    include_directories(${OPENGL_INCLUDE_DIRS})
    add_library(tvhal-xgl SHARED ${X64_SRCS} ../common/graph_xgl.c)
    target_link_libraries(tvhal-xgl ${X11_LIBRARIES} ${PIXMAN_LIBRARIES} ${OPENGL_LIBRARIES})
    list(APPEND X64PORTS tvhal-xgl)
endif()
if(XCB_FOUND AND XCB_IMAGE_LIBRARY)
    include_directories(${XCB_INCLUDE_DIR})
    add_library(tvhal-xcb SHARED ${X64_SRCS} ../common/graph_xcb.cc)
    target_link_libraries(tvhal-xcb ${X11_LIBRARIES} ${PIXMAN_LIBRARIES})
    add_library(tvhal SHARED ${X64_SRCS} ../common/graph_xcb.cc)
    target_link_libraries(tvhal PRIVATE ${XCB_LIBRARIES} ${XCB_IMAGE_LIBRARY})
    list(APPEND X64PORTS tvhal-xcb tvhal)
endif()
if(X11_FOUND)
    include_directories(${X11_INCLUDE_DIR}) 
    add_library(tvhal-x11 SHARED ${X64_SRCS} ../common/graph_xlib.c)
    target_link_libraries(tvhal-x11 ${X11_LIBRARIES} ${PIXMAN_LIBRARIES})
    list(APPEND X64PORTS tvhal-x11)
    if(NOT XCB_FOUND)
        add_library(tvhal SHARED ${X64_SRCS} ../common/graph_xlib.c)
        target_link_libraries(tvhal PRIVATE ${X11_LIBRARIES} ${PIXMAN_LIBRARIES})
        list(APPEND X64PORTS tvhal)
    endif()
endif(X11_FOUND)

if(DRM_FOUND)
    include_directories(${DRM_INCLUDE_DIRS})
    add_library(tvhal-drm SHARED ${X64_SRCS} ../common/graph_drm.cc)
    target_link_libraries(tvhal-drm ${DRM_LIBRARIES})
    list(APPEND X64PORTS tvhal-drm)
endif()


if(NOT X64PORTS)
    message(FATAL_ERROR "graph must implemented")
    find_package(ZLIB)
    add_definitions(-DENABLE_RFB=1)
    list(APPEND GRAPH_SRCS graph_rfb.c ../common/rfbcommon.cc)
    list(APPEND X64_LIBRARIES vncserver)
endif()

include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${X64_INCLUDE_DIRS}
)
message("CMAKE_SOURCE_DIR=${CMAKE_SOURCE_DIR} GRAPH_SRCS=${GRAPH_SRCS}")

set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(NONPKG_LIBS "-L${CMAKE_INSTALL_PREFIX}/lib")
configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
install (TARGETS ${X64PORTS}  DESTINATION lib)

