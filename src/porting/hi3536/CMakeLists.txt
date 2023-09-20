project (hi3536 C CXX)

set(HI3536_SRCS
	../common/cdlog.cc
	../common/input_linux.cc
	#mediaplayer.c
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

list(APPEND HI3536_INCLUDE_DIRS ../common)
list(APPEND HI3536_INCLUDE_DIRS ../include)
    
list(APPEND HI3536_SRCS hisi_sys.c graph_hisi.c)
list(APPEND HI3536_INCLUDE_DIRS ./include)
list(APPEND HI3536_LIBS -Wl,--start-group hdmi tde jpeg6b VoiceEngine dnvqe upvqe mpi -Wl,--end-group)

link_directories(${PROJECT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/lib)
list(APPEND HI3536_LIBS pthread)
include_directories(./
    ../common ../include
    ${PIXMAN_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${HI3536_INCLUDE_DIRS}
)
set(PREFIX ${CMAKE_INSTALL_PREFIX})

#configure_file(cdroidhal.pc.in cdroidhal.pc @ONLY)
#install(FILES ${CMAKE_CURRENT_BINARY_DIR}/cdroidhal.pc
#	DESTINATION ${CMAKE_INSTALL_PREFIX}/lib/pkgconfig)
add_library(tvhal SHARED ${HI3536_SRCS} )
target_link_libraries(tvhal PRIVATE ${HI3536_LIBS})
install (TARGETS tvhal  DESTINATION lib)
install (FILES ${MILIBS} DESTINATION lib)
