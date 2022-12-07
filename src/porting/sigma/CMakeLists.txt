project (sigma C CXX)

set(SIGMA_SRCS ../common/cdlog.cc 
	input_linux.cc  )

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

if(TRUE)
    list(APPEND SIGMA_SRCS graph_gfx.c)
else()
    list(APPEND SIGMA_SRCS graph_rfb.c)
    list(APPEND SIGMA_LIBS vncserver)
endif()

link_directories(${PROJECT_SOURCE_DIR}/libs)
list(APPEND SIGMA_LIBS pthread 
	-L${PROJECT_SOURCE_DIR}/libs -lmi_common -lmi_sys -lmi_gfx)
	#mi_common mi_sys mi_gfx)
find_package(ZLIB)
include_directories(./
    ../common ./inc ../include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${ZLIB_INCLUDE_DIR}
    ${CMAKE_BINARY_DIR}/include
)
message("PROJECT_SOURCE_DIR=${PROJECT_SOURCE_DIR}")
add_library(tvhal SHARED ${SIGMA_SRCS} )
target_link_libraries(tvhal ${SIGMA_LIBS})
install (TARGETS tvhal  DESTINATION lib)
