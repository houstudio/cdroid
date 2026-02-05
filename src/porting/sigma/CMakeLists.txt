project (sigma C CXX)

set(SIGMA_SRCS ../common/cdlog.cc
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

list(APPEND SIGMA_INCLUDE_DIRS ../common)
list(APPEND SIGMA_INCLUDE_DIRS ../include)


list(APPEND SIGMA_INCLUDE_DIRS ./inc)

link_directories(${PROJECT_SOURCE_DIR}/libs)
list(APPEND SIGMA_LIBS pthread dl
	-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_gfx 
	-lmi_disp -lmi_ai -lmi_ao -lmi_vdec -lmi_divp -lmi_panel
        -lhvplayer -lavformat -lavcodec -lavutil -lswresample -lswscale)
list(APPEND SIGMATARGET tvhal)

include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${SIGMA_INCLUDE_DIRS}
)
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(NONPKG_LIBS "-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_gfx \
 -lmi_disp -lmi_ai -lmi_ao -lmi_vdec -lmi_divp -lmi_panel \
 -lhvplayer -lavformat -lavcodec -lavutil -lswresample -lswscale")

configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
add_library(tvhal SHARED ${SIGMA_SRCS} graph_gfx.c)
target_link_libraries(tvhal ${SIGMA_LIBS})

add_library(tvhal-fb SHARED ${SIGMA_SRCS} ../common/graph_fb.c)
target_link_libraries(tvhal-fb ${SIGMA_LIBS})
add_dependencies(tvhal tvhal-fb)

file(GLOB MILIBS ${PROJECT_SOURCE_DIR}/libs/*.so)
install (TARGETS ${SIGMATARGET}  DESTINATION lib)
install (FILES ${MILIBS} DESTINATION lib)
