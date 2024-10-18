project (win32 C CXX)

set(WIN32_SRCS
     wininput.cc
     graph_win32.c
     ../common/cdlog.cc
     #../common/input_linux.cc
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
find_package(SDL2)
find_package(DirectFB)
if (SDL2_FOUND)
    include_directories(${SDL2_INCLUDE_DIRS})
    list(APPEND WIN32_SRCS ../common/graph_sdl.c)
    list(APPEND WIN32DEP_LIBRARIES ${SDL2_LIBRARIES})
endif(SDL2_FOUND)

include_directories(./
    ../common ../include
    ${CMAKE_SOURCE_DIR}/src/gui/
    ${CMAKE_SOURCE_DIR}/src/gui/core
    ${WIN32_INCLUDE_DIRS}
)

add_library(tvhal SHARED ${WIN32_SRCS})
if(MSVC)
    set_target_properties(tvhal PROPERTIES WINDOW_EXPORT_ALL_SYMBOLS ON)       
    list(APPEND WIN32DEP_LIBRARIES kernel32 gdi32)
endif()
target_link_libraries(tvhal ${WIN32DEP_LIBRARIES})

install (TARGETS ${WIN32PORTS}  DESTINATION lib)

