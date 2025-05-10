project (android C CXX)

set(ANDROID_SRCS
     ../common/input_linux.cc
     cdlog.cc
     cdplayer.c
     nativeapp.cc
     #misc.c
     #./android_native_app_glue.c
    )

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
find_package(Pixman)
find_package(OpenGL)

include_directories(../common ../include ${PIXMAN_INCLUDE_DIRS})

add_library(tvhal SHARED ${ANDROID_SRCS} ../common/graph_fb.c)
target_link_libraries(tvhal log android EGL GLESv2)

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
#configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
        DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
install (TARGETS ${X64PORTS}  DESTINATION lib)

