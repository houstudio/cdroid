project (sigma2351 C CXX)

set(SIGMA_SRCS ../common/cdlog.cc
        ../common/input_linux.cc
        mediaplayer.c)
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

set(SYSROOT_PATH /home/chenyang/2351-sdk/ssd235x_proj_all_in_one)
link_directories(${PROJECT_SOURCE_DIR}/libs)
list(APPEND SIGMA2351_LIBS pthread dl
	-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_scl -lmi_disp -lmi_gfx -lcam_fs_wrapper -lcam_os_wrapper
    -lcus3a -lispalgo -lmi_isp)
list(APPEND SIGMATARGET tvhal)

include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${SIGMA_INCLUDE_DIRS}
)
set(PREFIX ${CMAKE_INSTALL_PREFIX})
set(NONPKG_LIBS "-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_scl -lmi_disp -lmi_gfx -lcam_fs_wrapper -lcam_os_wrapper \
    -lcus3a -lispalgo  -lmi_isp")

#configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
#	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
add_library(tvhal SHARED ${SIGMA_SRCS} graph_gfx.c)
target_link_libraries(tvhal ${SIGMA2351_LIBS})

add_library(tvhal-fb SHARED ${SIGMA_SRCS} ../common/graph_fb.c)
target_link_libraries(tvhal-fb ${SIGMA2351_LIBS})
add_dependencies(tvhal tvhal-fb)

if(DIRECTFB_FOUND)
    add_library(tvhal-dfb SHARED ${SIGMA_SRCS} ../common/graph_dfb.c)
    include_directories(${DIRECTFB_INCLUDE_DIRS})
    target_link_libraries(tvhal-dfb ${DIRECTFB_LIBRARIES} ${SIGMA2351_LIBS})
    list(APPEND SIGMATARGET tvhal-dfb)
    add_dependencies(tvhal tvhal-dfb)
endif(DIRECTFB_FOUND)

file(GLOB MILIBS ${PROJECT_SOURCE_DIR}/libs/*.so)
install (TARGETS ${SIGMATARGET}  DESTINATION lib)
install (FILES ${MILIBS} DESTINATION lib)
