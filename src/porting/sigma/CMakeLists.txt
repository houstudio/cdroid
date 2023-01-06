project (sigma C CXX)

set(SIGMA_SRCS ../common/cdlog.cc 
	#mediaplayer.c 
	input_linux.cc  )
find_package(Pixman)
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

list(APPEND SIGMA_LIBS ${PIXMAN_LIBRARY})
if(TRUE)
    list(APPEND SIGMA_SRCS graph_gfx.c)
else()
    list(APPEND SIGMA_SRCS graph_rfb.c)
    list(APPEND SIGMA_LIBS vncserver)
endif()

link_directories(${PROJECT_SOURCE_DIR}/libs)
list(APPEND SIGMA_LIBS pthread 
	-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_gfx 
	)#-lmi_disp -lmi_ai -lmi_ao -lmi_vdec -lmi_divp -lmi_panel
        #-lhvplayer -lavformat -lavcodec -lavutil -lswresample -lswscale)

include_directories(./
    ../common ./inc ../include
    ${PIXMAN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
)
set(PREFIX ${CMAKE_INSTALL_PREFIX}/lib)
set(NONPKG_LIBS "-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_gfx \
 -lmi_disp -lmi_ai -lmi_ao -lmi_vdec -lmi_divp -lmi_panel \
 -lhvplayer -lavformat -lavcodec -lavutil -lswresample -lswscale")

configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
add_library(tvhal SHARED ${SIGMA_SRCS} )
target_link_libraries(tvhal PRIVATE ${SIGMA_LIBS})
file(GLOB MILIBS ${PROJECT_SOURCE_DIR}/libs/*.so)
install (TARGETS tvhal  DESTINATION lib)
install (FILES ${MILIBS} DESTINATION lib)
